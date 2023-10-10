#include "UConverterEnvelopeData.hpp"
#include "UConverterShapeData.hpp"
#include "J3DUtil.hpp"

#include <bstream.h>
#include <tiny_gltf.h>
#include <glm/glm.hpp>

J3D::Cnv::UConverterEnvelopeData::UConverterEnvelopeData() {

}

J3D::Cnv::UConverterEnvelopeData::~UConverterEnvelopeData() {

}

void J3D::Cnv::UConverterEnvelopeData::ProcessEnvelopes(const std::vector<UConverterShape*>& shapes) {
    // Fill unskinned indices first
    for (UConverterShape* shape : shapes) {
        for (UConverterPrimitive* prim : shape->GetPrimitives()) {
            for (UConverterVertex* v : prim->mVertices) {
                if (v->SkinInfo.JointIndices.size() == 1) {
                    uint16_t jointIndex = UINT16_MAX;

                    const auto itr = std::find(mUnskinnedIndices.begin(), mUnskinnedIndices.end(), v->SkinInfo.JointIndices[0]);
                    if (itr == mUnskinnedIndices.end()) {
                        jointIndex = mUnskinnedIndices.size();
                        mUnskinnedIndices.push_back(v->SkinInfo.JointIndices[0]);
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
    for (UConverterShape* shape : shapes) {
        for (UConverterPrimitive* prim : shape->GetPrimitives()) {
            for (UConverterVertex* v : prim->mVertices) {
                if (v->SkinInfo.JointIndices.size() != 1) {
                    UConverterEnvelope env = { v->SkinInfo.JointIndices, v->SkinInfo.Weights };
                    uint16_t envelopeIndex = UINT16_MAX;

                    const auto envelopeItr = std::find(mEnvelopes.begin(), mEnvelopes.end(), env);
                    if (envelopeItr == mEnvelopes.end()) {
                        envelopeIndex = mEnvelopes.size();
                        mEnvelopes.push_back(env);
                    }
                    else {
                        envelopeIndex = envelopeItr - mEnvelopes.begin();
                    }

                    uint16_t indexIndex = UINT16_MAX;

                    const auto indexItr = std::find(mSkinnedIndices.begin(), mSkinnedIndices.end(), envelopeIndex);
                    if (indexItr == mSkinnedIndices.end()) {
                        indexIndex = mSkinnedIndices.size();
                        mSkinnedIndices.push_back(envelopeIndex);
                    }
                    else {
                        indexIndex = indexItr - mSkinnedIndices.begin();
                    }

                    v->PosMatrixIndex = mUnskinnedIndices.size() + indexIndex;
                }
            }
        }
    }
}

void J3D::Cnv::UConverterEnvelopeData::ReadInverseBindMatrices(const tinygltf::Model* model, std::vector<bStream::CMemoryStream>& buffers) {
    // No skins or envelopes means we don't need to read the IBMs
    if (model->skins.size() == 0 || mEnvelopes.size() == 0) {
        return;
    }

    tinygltf::Skin skin = model->skins[0];

    const auto& ibmAccessor = model->accessors[skin.inverseBindMatrices];
    const auto& ibmView = model->bufferViews[ibmAccessor.bufferView];

    auto& ibmStream = buffers[ibmView.buffer];
    ibmStream.seek(ibmView.byteOffset);

    for (uint32_t i = 0; i < ibmAccessor.count; i++) {
        glm::mat4 ibm;

        ibm[0][0] = ibmStream.readFloat();
        ibm[1][0] = ibmStream.readFloat();
        ibm[2][0] = ibmStream.readFloat();
        ibm[3][0] = ibmStream.readFloat();

        ibm[0][1] = ibmStream.readFloat();
        ibm[1][1] = ibmStream.readFloat();
        ibm[2][1] = ibmStream.readFloat();
        ibm[3][1] = ibmStream.readFloat();

        ibm[0][2] = ibmStream.readFloat();
        ibm[1][2] = ibmStream.readFloat();
        ibm[2][2] = ibmStream.readFloat();
        ibm[3][2] = ibmStream.readFloat();

        ibm[0][3] = ibmStream.readFloat();
        ibm[1][3] = ibmStream.readFloat();
        ibm[2][3] = ibmStream.readFloat();
        ibm[3][3] = ibmStream.readFloat();

        mInverseBindMatrices.push_back(ibm);
    }
}

void J3D::Cnv::UConverterEnvelopeData::WriteEVP1(bStream::CStream& stream) {
    size_t streamStartPos = stream.tell();

    // Header
    stream.writeUInt32(0x45565031);        // FourCC ('EVP1')
    stream.writeUInt32(0);                 // Placeholder for section size
    stream.writeUInt16(mEnvelopes.size()); // Number of envelopes
    stream.writeUInt16(UINT16_MAX);        // Padding
    
    // Offsets
    stream.writeUInt32(0); // Placeholder for envelope element count array offset
    stream.writeUInt32(0); // Placeholder for envelope joint index array offset
    stream.writeUInt32(0); // Placeholder for envelope weight array offset
    stream.writeUInt32(0); // Placeholder for inverse bind matrix array offset

    if (mEnvelopes.size() != 0) {
        // Write element counts offset
        J3DUtility::WriteOffset(&stream, streamStartPos, 0x0C);
        // Write element counts
        for (const UConverterEnvelope& e : mEnvelopes) {
            stream.writeUInt8(e.JointIndices.size());
        }

        // Write joint indices offset
        J3DUtility::WriteOffset(&stream, streamStartPos, 0x10);
        // Write joint indices
        for (const UConverterEnvelope& e : mEnvelopes) {
            for (const uint16_t i : e.JointIndices) {
                stream.writeUInt16(i);
            }
        }

        // Write weights offset
        J3DUtility::WriteOffset(&stream, streamStartPos, 0x14);
        // Write weights
        for (const UConverterEnvelope& e : mEnvelopes) {
            for (const float f : e.Weights) {
                stream.writeFloat(f);
            }
        }

        // Write inverse bind matrices offset
        J3DUtility::WriteOffset(&stream, streamStartPos, 0x18);
        // Write inverse bind matrices
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
    }

    J3DUtility::PadStreamWithString(&stream, 32);

    // Write section size
    J3DUtility::WriteOffset(&stream, streamStartPos, 4);
}

void J3D::Cnv::UConverterEnvelopeData::WriteDRW1(bStream::CStream& stream) {
    size_t streamStartPos = stream.tell();

    // Header
    stream.writeUInt32(0x44525731);                                            // FourCC ('DRW1')
    stream.writeUInt32(0);                                                     // Placeholder for section size
    stream.writeUInt16(mUnskinnedIndices.size() + mSkinnedIndices.size() * 2); // Number of elements
    stream.writeUInt16(UINT16_MAX);                                            // Padding

    // Offsets
    stream.writeUInt32(0); // Placeholder for unskinned vs skinned boolean array offset
    stream.writeUInt32(0); // Placeholder for index array offset

    // Write unskinned vs skinned boolean array offset
    J3DUtility::WriteOffset(&stream, streamStartPos, 0x0C);
    // Write unskinned vs skinned boolean array
    for (uint32_t i = 0; i < mUnskinnedIndices.size(); i++) {
        stream.writeUInt8(0);
    }
    for (uint32_t i = 0; i < mSkinnedIndices.size(); i++) {
        stream.writeUInt8(1);
    }
    /* SIC: Nintendo's original tools erroneously duplicated the skinned indices */
    for (uint32_t i = 0; i < mSkinnedIndices.size(); i++) {
        stream.writeUInt8(1);
    }

    J3DUtility::PadStreamWithString(&stream, 2);

    // Write index array offset
    J3DUtility::WriteOffset(&stream, streamStartPos, 0x10);
    // Write index array
    for (const uint16_t& i : mUnskinnedIndices) {
        stream.writeUInt16(i);
    }
    for (const uint16_t& i : mSkinnedIndices) {
        stream.writeUInt16(i);
    }
    /* SIC: See previous note */
    for (const uint16_t& i : mSkinnedIndices) {
        stream.writeUInt16(i);
    }

    J3DUtility::PadStreamWithString(&stream, 32);

    // Write section size
    J3DUtility::WriteOffset(&stream, streamStartPos, 4);
}
