#ifndef __Asset_h__
#define __Asset_h__

#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MString.h>

#include <HAPI/HAPI.h>

#include "Object.h"

class AssetInputs;

class Asset {
    public:
        Asset(MString otlFilePath, MObject node);
        ~Asset();

        void init();

        Object** getObjects();
        Object* findObjectByName(MString name);
        Object* findObjectById(int id);

        // Getters for infos
        HAPI_ObjectInfo getObjectInfo(int id);        

        MStatus compute(const MPlug& plug, MDataBlock& data);

        MIntArray getParmIntValues(HAPI_ParmInfo& parm);
        MFloatArray getParmFloatValues(HAPI_ParmInfo& parm);
        MStringArray getParmStringValues(HAPI_ParmInfo& parm);

    public:
        HAPI_AssetInfo 	myAssetInfo;
        HAPI_NodeInfo	myNodeInfo;
        int myNumVisibleObjects;
        int myNumObjects;


    private:

        void update();

        void computeAssetInputs(const MPlug& plug, MDataBlock& data);
        void computeInstancerObjects(const MPlug& plug, MDataBlock& data);
        void computeGeometryObjects(const MPlug& plug, MDataBlock& data);


    private:
        MObject myNode;		    //The Maya asset node

	AssetInputs* myAssetInputs;
	//TODO: make this a vector.  The double pointer assumes the number of objects is static
        Object** myObjects;	    //the Object class contains a 1 to 1 map with HAPI_ObjectInfos.

        // Arrays of infos that can be accessed when updating objects,
        // keeping them here avoids getting them for individual object.
        HAPI_ObjectInfo* myObjectInfos;       
};

#endif
