#include "UConverterObject.hpp"

#include "J3DNameTable.hpp"
#include "J3DUtil.hpp"

#include <bstream.h>
#include <tiny_gltf.h>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>

J3D::Cnv::UConverterObject::UConverterObject() {

}

J3D::Cnv::UConverterObject::~UConverterObject() {
    mBufferStreams.clear();

    for (uint8_t* buffer : mBuffers) {
        delete[] buffer;
    }
    mBuffers.clear();

    for (UConverterMesh* mesh : mMeshes) {
        delete mesh;
    }
    mMeshes.clear();
}

void J3D::Cnv::UConverterObject::WriteTEX1(bStream::CStream& stream, tinygltf::Model* model) {
    const auto& textures = model->textures;
    size_t startingStreamPos = stream.tell();

    stream.writeUInt32(0x54455831);
    stream.writeUInt32(0);

    stream.writeUInt16(model->textures.size());
    stream.writeUInt16(UINT16_MAX);

    if (model->textures.size() == 0) {
        stream.writeUInt32(0);
        stream.writeUInt32(0x20);

        J3DUtility::PadStreamWithString(&stream, 16);

        stream.writeUInt16(0);
        stream.writeUInt16(UINT16_MAX);
        J3DUtility::PadStreamWithString(&stream, 16);

        return;
    }
}

void J3D::Cnv::UConverterObject::LoadBuffers(tinygltf::Model* model) {
    for (const auto& buf : model->buffers) {
        uint8_t* buffer = new uint8_t[buf.data.size()];
        std::memcpy(buffer, buf.data.data(), buf.data.size());

        mBuffers.push_back(buffer);
        mBufferStreams.push_back(bStream::CMemoryStream(buffer, buf.data.size(), bStream::Little, bStream::In));
    }
}



bool J3D::Cnv::UConverterObject::Load(tinygltf::Model* model) {
    LoadBuffers(model);

    mSkeletonData.BuildSkeleton(model);
    
    mShapeData.BuildVertexData(model, &mVertexData, mBufferStreams);

    mEnvelopeData.ProcessEnvelopes(mShapeData.GetShapes());
    mEnvelopeData.ReadInverseBindMatrices(model, mBufferStreams);

    return true;
}

bool J3D::Cnv::UConverterObject::WriteBMD(bStream::CStream& stream) {
    // Write header
    stream.writeUInt32(0x4A334432); // J3D fourCC ('J3D2')
    stream.writeUInt32(0x626D6433); // BMD fourCC ('bmd3')
    stream.writeUInt32(0);          // Placeholder for file size
    stream.writeUInt32(8);          // Number of BMD sections

    // Write padding area
    for (uint32_t i = 0; i < 0x10; i++) {
        stream.writeUInt8(UINT8_MAX);
    }

    // Write scenegraph
    mSkeletonData.WriteINF1(stream, mVertexData.GetVertexCount());

    // Write vertex data
    mVertexData.WriteVTX1(stream);

    // Write skinning envelope data
    mEnvelopeData.WriteEVP1(stream);
    mEnvelopeData.WriteDRW1(stream);

    // Write joint data
    mSkeletonData.WriteJNT1(stream);

    // Write geometry data
    //WriteSHP1(stream);
    
    // Write material data
    //WriteMAT3(stream);

    // Write texture data
    //WriteTEX1(stream);

    // Write file size
    J3DUtility::WriteOffset(&stream, 0, 8);

    return true;
}