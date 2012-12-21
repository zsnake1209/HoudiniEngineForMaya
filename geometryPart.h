#ifndef GEOMETRY_PART_H
#define GEOMETRY_PART_H

class GeometryPart
{
    public:
        GeometryPart();
        GeometryPart(int assetId, int objectId, int geoId, int partId,
                MString partName, HAPI_GeoInfo geoInfo, Asset* objectControl);
        virtual ~GeometryPart();


        virtual MStatus compute(MDataHandle& handle);

        virtual void setGeoInfo(HAPI_GeoInfo& info);
        virtual bool hasMesh();

    public:
        MString myPartName;

    protected:
        virtual void update();

    private:
        void updateMaterial(MDataHandle& handle);
        void updateFaceCounts();
        void updateVertexList();
        void updatePoints();
        void updateNormals();
        void updateUVs();

        MObject createMesh();

        // Utility
        virtual MFloatArray getAttributeFloatData(HAPI_AttributeOwner owner, MString name);

    private:
        Asset* myObjectControl;

        int myAssetId;
        int myObjectId;
        int myGeoId;
        int myPartId;

        HAPI_GeoInfo myGeoInfo;
        HAPI_PartInfo myPartInfo;
        HAPI_MaterialInfo myMaterialInfo;

        MIntArray myFaceCounts;
        MIntArray myVertexList;
        MFloatPointArray myPoints;
        MVectorArray myNormals;
        MFloatArray myUs;
        MFloatArray myVs;

        bool myNeverBuilt;
};

#endif