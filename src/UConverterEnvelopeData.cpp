#include "UConverterEnvelopeData.hpp"
#include "UConverterMesh.hpp"
#include "J3DUtil.hpp"

#include <bstream.h>

J3D::Cnv::UConverterEnvelopeData::UConverterEnvelopeData() {

}

J3D::Cnv::UConverterEnvelopeData::~UConverterEnvelopeData() {

}

void J3D::Cnv::UConverterEnvelopeData::ProcessEnvelopes(const std::vector<UConverterMesh*>& meshes) {
    // Fill unskinned indices first
    for (UConverterMesh* mesh : meshes) {
        for (UConverterPrimitive* prim : mesh->GetPrimitives()) {
            for (UConverterVertex* v : prim->mVertices) {
                if (v->JointIndices.size() == 1) {
                    uint16_t jointIndex = UINT16_MAX;

                    const auto itr = std::find(mUnskinnedIndices.begin(), mUnskinnedIndices.end(), v->JointIndices[0]);
                    if (itr == mUnskinnedIndices.end()) {
                        jointIndex = mUnskinnedIndices.size();
                        mUnskinnedIndices.push_back(v->JointIndices[0]);
                    }
                    else {
                        jointIndex = itr - mUnskinnedIndices.begin();
                    }

                    v->PosMatrixIndex = jointIndex;
                }
            }
        }
    }

    // Fill skinned indices next
    for (UConverterMesh* mesh : meshes) {
        for (UConverterPrimitive* prim : mesh->GetPrimitives()) {
            for (UConverterVertex* v : prim->mVertices) {
                if (v->JointIndices.size() != 1) {
                    UConverterEnvelope env = { v->JointIndices, v->Weights };

                    uint16_t envelopeIndex = UINT16_MAX;
                    const auto itr = std::find(mEnvelopes.begin(), mEnvelopes.end(), env);
                    if (itr == mEnvelopes.end()) {
                        envelopeIndex = mEnvelopes.size();
                        mEnvelopes.push_back(env);
                    }
                    else {
                        envelopeIndex = itr - mEnvelopes.begin();
                    }

                    v->PosMatrixIndex = mUnskinnedIndices.size() + mSkinnedIndices.size();
                    mSkinnedIndices.push_back(envelopeIndex);
                }
            }
        }
    }
}

void J3D::Cnv::UConverterEnvelopeData::WriteEVP1(bStream::CStream& stream) {
    size_t streamStartPos = stream.tell();

    stream.writeUInt32(0x45565031);
    stream.writeUInt32(0);
    stream.writeUInt16(mEnvelopes.size());
    stream.writeUInt16(UINT16_MAX);
    stream.writeUInt32(0x1C);

    stream.writeUInt32(0);
    stream.writeUInt32(0);
    stream.writeUInt32(0);

    // Element counts
    for (const UConverterEnvelope& e : mEnvelopes) {
        stream.writeUInt8(e.JointIndices.size());
    }

    size_t currentStreamPos = stream.tell();

    stream.seek(streamStartPos + 0x10);
    stream.writeUInt32(static_cast<uint32_t>(currentStreamPos - streamStartPos));
    stream.seek(currentStreamPos);

    // Joint indices
    for (const UConverterEnvelope& e : mEnvelopes) {
        for (const uint16_t i : e.JointIndices) {
            stream.writeUInt16(i);
        }
    }

    J3DUtility::PadStreamWithString(&stream, 16);

    currentStreamPos = stream.tell();

    stream.seek(streamStartPos + 0x14);
    stream.writeUInt32(static_cast<uint32_t>(currentStreamPos - streamStartPos));
    stream.seek(currentStreamPos);

    // Weights
    for (const UConverterEnvelope& e : mEnvelopes) {
        for (const float f : e.Weights) {
            stream.writeFloat(f);
        }
    }

    currentStreamPos = stream.tell();

    stream.seek(streamStartPos + 0x18);
    stream.writeUInt32(static_cast<uint32_t>(currentStreamPos - streamStartPos));
    stream.seek(currentStreamPos);

    // Inverse bind matrices
    for (const glm::mat3x4& m : mInverseBindMatrices) {
        stream.writeFloat(m[0][0]);
        stream.writeFloat(m[0][1]);
        stream.writeFloat(m[0][2]);
        stream.writeFloat(m[0][3]);

        stream.writeFloat(m[1][0]);
        stream.writeFloat(m[1][1]);
        stream.writeFloat(m[1][2]);
        stream.writeFloat(m[1][3]);

        stream.writeFloat(m[2][0]);
        stream.writeFloat(m[2][1]);
        stream.writeFloat(m[2][2]);
        stream.writeFloat(m[2][3]);
    }

    J3DUtility::PadStreamWithString(&stream, 32);

    size_t streamEndPos = stream.tell();

    stream.seek(streamStartPos + 4);
    stream.writeUInt32(static_cast<uint32_t>(streamEndPos - streamStartPos));
    stream.seek(streamEndPos);
}

void J3D::Cnv::UConverterEnvelopeData::WriteDRW1(bStream::CStream& stream) {
    size_t streamStartPos = stream.tell();

    stream.writeUInt32(0x44525731);
    stream.writeUInt32(0);
    stream.writeUInt16(mUnskinnedIndices.size() + mSkinnedIndices.size());
    stream.writeUInt16(UINT16_MAX);

    stream.writeUInt32(0x14);
    stream.writeUInt32(0);

    // Bools indicating unskinned vs skinned
    for (uint32_t i = 0; i < mUnskinnedIndices.size(); i++) {
        stream.writeUInt8(0);
    }

    for (uint32_t i = 0; i < mSkinnedIndices.size(); i++) {
        stream.writeUInt8(1);
    }

    // SIC: Nintendo's original tools erroneously duplicated the skinned indices.
    for (uint32_t i = 0; i < mSkinnedIndices.size(); i++) {
        stream.writeUInt8(1);
    }

    J3DUtility::PadStreamWithString(&stream, 2);

    size_t currentStreamPos = stream.tell();

    stream.seek(streamStartPos + 0x10);
    stream.writeUInt32(static_cast<uint32_t>(currentStreamPos - streamStartPos));
    stream.seek(currentStreamPos);

    // Envelope indices
    for (const uint16_t& i : mUnskinnedIndices) {
        stream.writeUInt16(i);
    }

    for (const uint16_t& i : mSkinnedIndices) {
        stream.writeUInt16(i);
    }

    // SIC: See previous note.
    for (const uint16_t& i : mSkinnedIndices) {
        stream.writeUInt16(i);
    }

    J3DUtility::PadStreamWithString(&stream, 32);

    size_t streamEndPos = stream.tell();
    stream.seek(streamStartPos + 4);
    stream.writeUInt32(static_cast<uint32_t>(streamEndPos - streamStartPos));
    stream.seek(streamEndPos);
}
