#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <vector>

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

            uint8_t mMatrixType;

            std::vector<EGXAttribute> mEnabledAttributes;
            UConverterBoundingVolume mBounds;

        public:
            UConverterShape();
            ~UConverterShape();
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