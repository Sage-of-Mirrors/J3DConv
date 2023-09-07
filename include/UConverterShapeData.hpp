#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace bStream {
    class CMemoryStream;
}

namespace J3D {
    namespace Cnv {
        class UConverterVertexData;

        /* UConverterVertex */

        struct UConverterVertex {
            uint16_t PositionIndex = UINT16_MAX;

            // If this is set, then the mesh is using normal/binormal/tangent
            // rather than typical vertex normals.
            bool bUseNBT = false;
            uint16_t NormalIndex = UINT16_MAX;

            uint16_t ColorIndex[2]{ UINT16_MAX, UINT16_MAX };
            uint16_t TexCoordIndex[8]{
                UINT16_MAX,
                UINT16_MAX,
                UINT16_MAX,
                UINT16_MAX,
                UINT16_MAX,
                UINT16_MAX,
                UINT16_MAX,
                UINT16_MAX
            };

            uint16_t PosMatrixIndex = UINT16_MAX;

            std::vector<uint16_t> JointIndices;
            std::vector<float> Weights;

            void SetIndex(EGXAttribute attribute, uint16_t index);
        };

        /* UConverterPrimitive* */

        struct UConverterPrimitive {
            EGXPrimitiveType mPrimitiveType = EGXPrimitiveType::None;
            std::vector<UConverterVertex*> mVertices;

        public:
            UConverterPrimitive() { }
            ~UConverterPrimitive();
        };

        /* UConverterShape */

        class UConverterShape {
            std::vector<UConverterPrimitive*> mPrimitives;

            // Utility properties
            std::string mMaterialName = "";

            uint32_t mIndex = UINT32_MAX;
            uint32_t mMaterialIndex = UINT32_MAX;
            uint32_t mJointIndex = UINT32_MAX;

            // J3D properties
            uint8_t mMatrixType;
            std::vector<EGXAttribute> mEnabledAttributes;
            UConverterBoundingVolume mBounds;

        public:
            UConverterShape();
            ~UConverterShape();

            void CalculateBoundingVolume(const std::vector<glm::vec4>& positions);

            void AddPrimitive(UConverterPrimitive* prim) { if (prim != nullptr) mPrimitives.push_back(prim); }
            std::vector<UConverterPrimitive*>& GetPrimitives() { return mPrimitives; }

            const std::string& GetMaterialName() const { return mMaterialName; }

            uint32_t GetIndex() const { return mIndex; }
            uint32_t GetMaterialIndex() const { return mMaterialIndex; }
            uint32_t GetJointIndex() const { return mJointIndex; }

            void SetMaterialName(std::string name) { mMaterialName = name; }

            void SetIndex(uint32_t index) { mIndex = index; }
            void SetMaterialIndex(uint32_t index) { mMaterialIndex = index; }
            void SetJointIndex(uint32_t index) { mJointIndex = index; }
        };

        /* UConverterShape Data */

        class UConverterShapeData {
            std::vector<UConverterShape*> mShapes;

            void ReadGltfVertexAttribute(
                const tinygltf::Model* model,
                std::vector<bStream::CMemoryStream>& buffers,
                uint32_t attributeAccessorIndex,
                std::vector<glm::vec4>& values
            );
            void ReadGltfIndices(
                const tinygltf::Model* model,
                std::vector<bStream::CMemoryStream>& buffers,
                uint32_t indexAccessorIndex,
                std::vector<uint16_t>& indices
            );

        public:
            UConverterShapeData();
            ~UConverterShapeData();

            void BuildVertexData(
                tinygltf::Model* model,
                UConverterVertexData* vertexData,
                std::vector<bStream::CMemoryStream>& buffers
            );

            std::vector<UConverterShape*>& GetShapes() { return mShapes; }
        };
    }
}