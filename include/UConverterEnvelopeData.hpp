#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/mat3x4.hpp>

#include <vector>

namespace bStream {
    class CMemoryStream;
}

namespace tinygltf {
    class Model;
}

namespace J3D {
    namespace Cnv {
        class UConverterMesh;

        struct UConverterEnvelope {
            std::vector<uint16_t> JointIndices;
            std::vector<float> Weights;

            bool operator==(const UConverterEnvelope& other) {
                return JointIndices == other.JointIndices && Weights == other.Weights;
            }

            bool operator!=(const UConverterEnvelope& other) {
                return !operator==(other);
            }
        };

        class UConverterEnvelopeData {
            // EVP1 data
            std::vector<UConverterEnvelope> mEnvelopes;
            std::vector<glm::mat3x4> mInverseBindMatrices;

            // DRW1 data
            std::vector<uint16_t> mUnskinnedIndices;
            std::vector<uint16_t> mSkinnedIndices;

        public:
            UConverterEnvelopeData();
            ~UConverterEnvelopeData();

            void ProcessEnvelopes(const std::vector<UConverterMesh*>& meshes);
            void ReadInverseBindMatrices(const tinygltf::Model* model, std::vector<bStream::CMemoryStream>& buffers);

            void WriteEVP1(bStream::CStream& stream);
            void WriteDRW1(bStream::CStream& stream);
        };
    }
}
