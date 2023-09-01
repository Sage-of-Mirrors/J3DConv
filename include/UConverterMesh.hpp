#pragma once

#include "types.hpp"
#include "UConverterMesh.hpp"

#include <vector>

namespace bStream {
    class CMemoryStream;
}

namespace tinygltf {
    struct Mesh;
}

namespace J3D {
    namespace Cnv {
        struct UConverterVertex {
            uint16_t PositionIndex = UINT16_MAX;

            // If this is set, then the mesh is using normal/binormal/tangent
            // rather than typical vertex normals.
            bool bUseNBT = false;
            uint16_t NormalIndex = UINT16_MAX;

            uint16_t ColorIndex[2] { UINT16_MAX, UINT16_MAX };
            uint16_t TexCoordIndex[8] {
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

            std::vector<uint32_t> JointIndices;
            std::vector<float> Weights;

            void SetIndex(EGXAttribute attribute, uint16_t index);
        };

        struct UConverterPrimitive {
            EGXPrimitiveType mPrimitiveType = EGXPrimitiveType::None;
            std::vector<UConverterVertex*> mVertices;

        public:
            UConverterPrimitive() { }
            ~UConverterPrimitive();
        };

        class UConverterMesh {
            std::vector<UConverterPrimitive*> mPrimitives;

        public:
            UConverterMesh();
            ~UConverterMesh();

            void AddPrimitive(UConverterPrimitive* prim) { if (prim != nullptr) mPrimitives.push_back(prim); }
        };
    }
}
