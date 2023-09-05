#pragma once

#include "types.hpp"
#include "UConverterVertexData.hpp"
#include "UConverterEnvelopeData.hpp"
#include "UConverterSkeletonData.hpp"
#include "UConverterShapeData.hpp"

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
            UConverterSkeletonData mSkeletonData;
            UConverterEnvelopeData mEnvelopeData;
            UConverterShapeData mShapeData;

            void WriteTEX1(bStream::CStream& stream, tinygltf::Model* model);

            void LoadBuffers(tinygltf::Model* model);

        public:
            UConverterObject();
            ~UConverterObject();

            bool Load(tinygltf::Model* model);
            bool WriteBMD(bStream::CStream& stream);
        };
    }
}