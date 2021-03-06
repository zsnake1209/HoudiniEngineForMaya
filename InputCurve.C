#include "InputCurve.h"

#include <sstream>

#include <maya/MDataBlock.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>

#include <HAPI/HAPI.h>

#include "util.h"

InputCurve::InputCurve(int assetId, int inputIdx) :
    Input(assetId, inputIdx)
{
    Util::PythonInterpreterLock pythonInterpreterLock;

    int curveAssetId;
    CHECK_HAPI(HAPI_CreateCurve(
                Util::theHAPISession.get(),
                &curveAssetId
                ));
    if(!Util::statusCheckLoop())
    {
        DISPLAY_ERROR(MString("Unexpected error when creating input curve."));
    }

    HAPI_GetAssetInfo(
            Util::theHAPISession.get(),
            curveAssetId,
            &myCurveAssetInfo
            );
    HAPI_GetNodeInfo(
            Util::theHAPISession.get(),
            myCurveAssetInfo.nodeId,
            &myCurveNodeInfo
            );
}

InputCurve::~InputCurve()
{
    HAPI_DisconnectAssetGeometry(
            Util::theHAPISession.get(),
            myAssetId,
            myInputIdx
            );

    HAPI_DestroyAsset(
            Util::theHAPISession.get(),
            myCurveAssetInfo.id
            );
}

InputCurve::AssetInputType
InputCurve::assetInputType() const
{
    return Input::AssetInputType_Curve;
}

void
InputCurve::setInputTransform(MDataHandle &dataHandle)
{
    MMatrix transformMatrix = dataHandle.asMatrix();

    float matrix[16];
    transformMatrix.get(reinterpret_cast<float(*)[4]>(matrix));

    HAPI_TransformEuler transformEuler;
    HAPI_ConvertMatrixToEuler(
            Util::theHAPISession.get(),
            matrix,
            HAPI_SRT,
            HAPI_XYZ,
            &transformEuler
            );
    HAPI_SetObjectTransform(
            Util::theHAPISession.get(),
            myCurveAssetInfo.id,
            0,
            &transformEuler
            );
}

void
InputCurve::setInputGeo(
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    HAPI_ConnectAssetGeometry(
            Util::theHAPISession.get(),
            myCurveAssetInfo.id, 0,
            myAssetId, myInputIdx
            );

    MDataHandle dataHandle = dataBlock.inputValue(plug);

    MObject curveObj = dataHandle.asNurbsCurve();
    if(curveObj.isNull())
    {
        return;
    }

    // find coords parm
    std::vector<HAPI_ParmInfo> parms(myCurveNodeInfo.parmCount);
    HAPI_GetParameters(
            Util::theHAPISession.get(),
            myCurveNodeInfo.id,
            &parms[0],
            0, myCurveNodeInfo.parmCount
            );
    int typeParmIndex = Util::findParm(parms, "type");
    int coordsParmIndex = Util::findParm(parms, "coords");
    int orderParmIndex = Util::findParm(parms, "order");
    int closeParmIndex = Util::findParm(parms, "close");
    if(coordsParmIndex < 0
            || coordsParmIndex < 0
            || orderParmIndex < 0
            || closeParmIndex < 0)
    {
        return;
    }

    const HAPI_ParmInfo &typeParm = parms[typeParmIndex];
    const HAPI_ParmInfo &coordsParm = parms[coordsParmIndex];
    const HAPI_ParmInfo &orderParm = parms[orderParmIndex];
    const HAPI_ParmInfo &closeParm = parms[closeParmIndex];

    MFnNurbsCurve curveFn(curveObj);

    // type
    {
        const char *type = "nurbs";
        if(curveObj.apiType() == MFn::kBezierCurveData)
        {
            type = "bezier";
        }

        HAPI_ParmChoiceInfo* choices = new HAPI_ParmChoiceInfo[typeParm.choiceCount];
        HAPI_GetParmChoiceLists(
                Util::theHAPISession.get(),
                myCurveNodeInfo.id,
                choices,
                typeParm.choiceIndex, typeParm.choiceCount
                );

        int nurbsIdx = -1;
        for(int i = 0; i < typeParm.choiceCount; i++)
        {
            if(Util::HAPIString(choices[i].valueSH) == type)
            {
                nurbsIdx = i;
                break;
            }
        }

        delete [] choices;

        if(nurbsIdx < 0)
        {
            return;
        }

        HAPI_SetParmIntValues(
                Util::theHAPISession.get(),
                myCurveNodeInfo.id,
                &nurbsIdx,
                typeParm.intValuesIndex,
                1
                );
    }

    // coords
    {
        MPointArray cvs;
        curveFn.getCVs(cvs);

        // Maya has curveFn.degree() more cvs in it's data definition
        // than houdini for periodic curves--but they are conincident
        // with the first ones. Houdini ignores them, so we don't
        // output them.
        int num_houdini_cvs = cvs.length();
        if(curveFn.form() == MFnNurbsCurve::kPeriodic)
            num_houdini_cvs -= curveFn.degree();

        std::ostringstream coords;
        for(unsigned int i = 0; i < (unsigned int) num_houdini_cvs; i++)
        {
            const MPoint &pt = cvs[i];

            coords << pt.x << "," << pt.y << "," << pt.z << " ";
        }
        HAPI_SetParmStringValue(
                Util::theHAPISession.get(),
                myCurveNodeInfo.id,
                coords.str().c_str(),
                coordsParm.id,
                coordsParm.stringValuesIndex
                );
    }

    // order
    {
        int order = curveFn.degree() + 1;

        HAPI_SetParmIntValues(
                Util::theHAPISession.get(),
                myCurveNodeInfo.id,
                &order,
                orderParm.intValuesIndex, 1
                );
    }

    // periodicity
    {
        int close = curveFn.form() == MFnNurbsCurve::kPeriodic;
        HAPI_SetParmIntValues(
                Util::theHAPISession.get(),
                myCurveNodeInfo.id,
                &close,
                closeParm.intValuesIndex, 1
                );
    }
}
