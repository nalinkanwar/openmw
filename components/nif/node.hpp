#ifndef OPENMW_COMPONENTS_NIF_NODE_HPP
#define OPENMW_COMPONENTS_NIF_NODE_HPP

#include <array>
#include <unordered_map>

#include <osg/Plane>

#include "base.hpp"

namespace Nif
{

    struct NiNode;

    struct NiBoundingVolume
    {
        enum Type : uint32_t
        {
            BASE_BV = 0xFFFFFFFF,
            SPHERE_BV = 0,
            BOX_BV = 1,
            CAPSULE_BV = 2,
            LOZENGE_BV = 3,
            UNION_BV = 4,
            HALFSPACE_BV = 5
        };

        struct NiBoxBV
        {
            osg::Vec3f center;
            Matrix3 axes;
            osg::Vec3f extents;
        };

        struct NiCapsuleBV
        {
            osg::Vec3f center, axis;
            float extent{ 0.f }, radius{ 0.f };
        };

        struct NiLozengeBV
        {
            float radius{ 0.f }, extent0{ 0.f }, extent1{ 0.f };
            osg::Vec3f center, axis0, axis1;
        };

        struct NiHalfSpaceBV
        {
            osg::Plane plane;
            osg::Vec3f origin;
        };

        uint32_t type{ BASE_BV };
        osg::BoundingSpheref sphere;
        NiBoxBV box;
        NiCapsuleBV capsule;
        NiLozengeBV lozenge;
        std::vector<NiBoundingVolume> children;
        NiHalfSpaceBV halfSpace;

        void read(NIFStream* nif);
    };

    struct NiSequenceStreamHelper : NiObjectNET
    {
    };

    // NiAVObject is an object that is a part of the main NIF tree. It has
    // a parent node (unless it's the root) and transformation relative to its parent.
    struct NiAVObject : public NiObjectNET
    {
        enum Flags
        {
            Flag_Hidden = 0x0001,
            Flag_MeshCollision = 0x0002,
            Flag_BBoxCollision = 0x0004,
            Flag_ActiveCollision = 0x0020
        };

        // Node flags. Interpretation depends on the record type.
        uint32_t mFlags;
        NiTransform mTransform;
        osg::Vec3f mVelocity;
        PropertyList mProperties;
        NiBoundingVolume mBounds;
        NiCollisionObjectPtr mCollision;
        // Parent nodes for the node. Only types derived from NiNode can be parents.
        std::vector<NiNode*> mParents;
        bool mIsBone{ false };

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;

        void setBone();
        bool isHidden() const { return mFlags & Flag_Hidden; }
        bool hasMeshCollision() const { return mFlags & Flag_MeshCollision; }
        bool hasBBoxCollision() const { return mFlags & Flag_BBoxCollision; }
        bool collisionActive() const { return mFlags & Flag_ActiveCollision; }
    };

    struct NiNode : NiAVObject
    {
        enum BSAnimFlags
        {
            AnimFlag_AutoPlay = 0x0020
        };

        enum BSParticleFlags
        {
            ParticleFlag_AutoPlay = 0x0020,
            ParticleFlag_LocalSpace = 0x0080
        };

        NiAVObjectList mChildren;
        NiAVObjectList mEffects;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct NiGeometry : NiAVObject
    {
        /* Possible flags:
            0x40 - mesh has no vertex normals ?

            Only flags included in 0x47 (ie. 0x01, 0x02, 0x04 and 0x40) have
            been observed so far.
        */

        struct MaterialData
        {
            std::vector<std::string> names;
            std::vector<int> extra;
            unsigned int active{ 0 };
            bool needsUpdate{ false };
            void read(NIFStream* nif);
        };

        NiGeometryDataPtr data;
        NiSkinInstancePtr skin;
        MaterialData material;
        BSShaderPropertyPtr shaderprop;
        NiAlphaPropertyPtr alphaprop;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct NiTriShape : NiGeometry
    {
    };
    struct BSLODTriShape : NiTriShape
    {
        unsigned int lod0, lod1, lod2;
        void read(NIFStream* nif) override;
    };
    struct NiTriStrips : NiGeometry
    {
    };
    struct NiLines : NiGeometry
    {
    };
    struct NiParticles : NiGeometry
    {
    };

    struct NiCamera : NiAVObject
    {
        struct Camera
        {
            unsigned short cameraFlags{ 0 };

            // Camera frustrum
            float left, right, top, bottom, nearDist, farDist;

            // Viewport
            float vleft, vright, vtop, vbottom;

            // Level of detail modifier
            float LOD;

            // Orthographic projection usage flag
            bool orthographic{ false };

            void read(NIFStream* nif);
        };
        Camera cam;

        void read(NIFStream* nif) override;
    };

    // A node used as the base to switch between child nodes, such as for LOD levels.
    struct NiSwitchNode : public NiNode
    {
        unsigned int switchFlags{ 0 };
        unsigned int initialIndex{ 0 };

        void read(NIFStream* nif) override;
    };

    struct NiLODNode : public NiSwitchNode
    {
        osg::Vec3f lodCenter;

        struct LODRange
        {
            float minRange;
            float maxRange;
        };
        std::vector<LODRange> lodLevels;

        void read(NIFStream* nif) override;
    };

    struct NiFltAnimationNode : public NiSwitchNode
    {
        float mDuration;
        enum Flags
        {
            Flag_Swing = 0x40
        };

        void read(NIFStream* nif) override;

        bool swing() const { return mFlags & Flag_Swing; }
    };

    // Abstract
    struct NiAccumulator : Record
    {
        void read(NIFStream* nif) override {}
    };

    // Node children sorters
    struct NiClusterAccumulator : NiAccumulator
    {
    };
    struct NiAlphaAccumulator : NiClusterAccumulator
    {
    };

    struct NiSortAdjustNode : NiNode
    {
        enum SortingMode
        {
            SortingMode_Inherit,
            SortingMode_Off,
            SortingMode_Subsort
        };

        int mMode;
        NiAccumulatorPtr mSubSorter;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct NiBillboardNode : NiNode
    {
        int mMode{ 0 };

        void read(NIFStream* nif) override;
    };

    struct NiDefaultAVObjectPalette : Record
    {
        NiAVObjectPtr mScene;
        std::unordered_map<std::string, NiAVObjectPtr> mObjects;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct BSTreeNode : NiNode
    {
        NiAVObjectList mBones1, mBones2;
        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct BSMultiBoundNode : NiNode
    {
        BSMultiBoundPtr mMultiBound;
        unsigned int mType{ 0 };

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct BSVertexDesc
    {
        unsigned char mVertexDataSize;
        unsigned char mDynamicVertexSize;
        unsigned char mUV1Offset;
        unsigned char mUV2Offset;
        unsigned char mNormalOffset;
        unsigned char mTangentOffset;
        unsigned char mColorOffset;
        unsigned char mSkinningDataOffset;
        unsigned char mLandscapeDataOffset;
        unsigned char mEyeDataOffset;
        unsigned short mFlags;

        enum VertexAttribute
        {
            Vertex = 0x0001,
            UVs = 0x0002,
            UVs_2 = 0x0004,
            Normals = 0x0008,
            Tangents = 0x0010,
            Vertex_Colors = 0x0020,
            Skinned = 0x0040,
            Land_Data = 0x0080,
            Eye_Data = 0x0100,
            Instance = 0x0200,
            Full_Precision = 0x0400,
        };

        void read(NIFStream* nif);
    };

    struct BSVertexData
    {
        osg::Vec3f mVertex;
        float mBitangentX;
        unsigned int mUnusedW;
        std::array<Misc::float16_t, 2> mUV;

        std::array<char, 3> mNormal;
        char mBitangentY;
        std::array<char, 3> mTangent;
        char mBitangentZ;
        std::array<char, 4> mVertColors;
        std::array<Misc::float16_t, 4> mBoneWeights;
        std::array<char, 4> mBoneIndices;
        float mEyeData;

        void read(NIFStream* nif, uint16_t flags);
    };

    struct BSTriShape : NiAVObject
    {
        osg::BoundingSpheref mBoundingSphere;
        std::array<float, 6> mBoundMinMax;

        NiSkinInstancePtr mSkin;
        BSShaderPropertyPtr mShaderProperty;
        NiAlphaPropertyPtr mAlphaProperty;

        BSVertexDesc mVertDesc;

        unsigned int mDataSize;
        unsigned int mParticleDataSize;

        std::vector<BSVertexData> mVertData;
        std::vector<unsigned short> mTriangles;
        std::vector<unsigned short> mParticleTriangles;
        std::vector<osg::Vec3f> mParticleVerts;
        std::vector<osg::Vec3f> mParticleNormals;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct BSValueNode : NiNode
    {
        unsigned int mValue;
        char mValueFlags;

        void read(NIFStream* nif) override;
    };

    struct BSOrderedNode : NiNode
    {
        osg::Vec4f mAlphaSortBound;
        char mStaticBound;

        void read(NIFStream* nif) override;
    };

    struct BSRangeNode : NiNode
    {
        uint8_t mMin, mMax;
        uint8_t mCurrent;

        void read(NIFStream* nif) override;
    };

}
#endif
