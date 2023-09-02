#pragma once

#include "types.hpp"
#include "UConverterVertexData.hpp"
#include "UConverterEnvelopeData.hpp"
#include "UConverterMesh.hpp"

#include <vector>

namespace bStream {
    class CMemoryStream;
}

namespace tinygltf {
    class Node;
    struct Skin;
}

namespace J3D {
    namespace Cnv {
        class UConverterObject {
            std::vector<uint8_t*> mBuffers;
            std::vector<bStream::CMemoryStream> mBufferStreams;

            UConverterVertexData mVertexData;
            UConverterEnvelopeData mEnvelopeData;
            std::vector<UConverterMesh*> mMeshes;

            void WriteJNT1(bStream::CStream& stream, tinygltf::Model* model);
            void WriteTEX1(bStream::CStream& stream, tinygltf::Model* model);

            void LoadBuffers(tinygltf::Model* model);
            void BuildVertexData(tinygltf::Model* model);

            void ReadGltfVertexAttribute(const tinygltf::Model* model, uint32_t attributeAccessorIndex, std::vector<glm::vec4>& values);
            void ReadGltfIndices(const tinygltf::Model* model, uint32_t indexAccessorIndex, std::vector<uint16_t>& indices);

        public:
            UConverterObject();
            ~UConverterObject();

            bool Load(tinygltf::Model* model);
        };
    }
}