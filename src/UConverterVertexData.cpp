#include "UConverterVertexData.hpp"
#include "UConverterMesh.hpp"
#include "J3DUtil.hpp"

#include <bstream.h>

#include <algorithm>

const uint8_t FIXED_POINT_EXP_NORMAL = 0x0E;
const uint8_t FIXED_POINT_EXP_TEXCOORD = 0x08;

J3D::Cnv::UConverterVertexData::UConverterVertexData() {

}

J3D::Cnv::UConverterVertexData::~UConverterVertexData() {

}

bool J3D::Cnv::UConverterVertexData::AttributeContainsValue(EGXAttribute attribute, const glm::vec4& value) {
    if (mVertexData.find(attribute) == mVertexData.end()) {
        return false;
    }

    const auto& attributeValues = mVertexData[attribute];
    return std::find(attributeValues.begin(), attributeValues.end(), value) != attributeValues.end();
}

uint16_t J3D::Cnv::UConverterVertexData::GetIndexOfValueInAttribute(EGXAttribute attribute, const glm::vec4& value) {
    if (mVertexData.find(attribute) == mVertexData.end()) {
        return UINT16_MAX;
    }

    const auto& attributeValues = mVertexData[attribute];
    const auto& valueItr = std::find(attributeValues.begin(), attributeValues.end(), value);

    if (valueItr == attributeValues.end()) {
        return UINT16_MAX;
    }

    return valueItr - attributeValues.begin();
}

uint16_t J3D::Cnv::UConverterVertexData::AddValueToAttribute(EGXAttribute attribute, const glm::vec4& value) {
    mVertexData[attribute].push_back(value);
    return mVertexData[attribute].size() - 1;
}

void J3D::Cnv::UConverterVertexData::BuildConverterPrimitive(const std::map<EGXAttribute, std::vector<glm::vec4>>& attributes,
                                                             const std::vector<uint16_t>& indices,
                                                             const std::vector<glm::vec4>& jointIndices,
                                                             const std::vector<glm::vec4>& jointWeights,
                                                             UConverterPrimitive* primitive) {
    for (uint32_t i = 0; i < indices.size(); i++) {
        uint16_t vertexIndex = indices[i];
        UConverterVertex* cnvVertex = new UConverterVertex();

        if (attributes.at(EGXAttribute::NBT).size() != 0) {
            cnvVertex->bUseNBT = true;
        }

        // Record vertex weight information for later
        for (uint32_t j = 0; j < 4; j++) {
            if (jointWeights[vertexIndex][j] != 0.0f) {
                cnvVertex->JointIndices.push_back(jointIndices[vertexIndex][j]);
                cnvVertex->Weights.push_back(jointWeights[vertexIndex][j]);
            }
        }

        // Strip vertex attributes to only unique values, and set the vertex indices to match
        for (const auto& [attribute, values] : attributes) {
            if (attribute == EGXAttribute::NBT) {
                ProcessNBTData(values, vertexIndex, cnvVertex);
                continue;
            }

            uint16_t newIndex = GetIndexOfValueInAttribute(attribute, values[vertexIndex]);

            if (newIndex == UINT16_MAX) {
                newIndex = AddValueToAttribute(attribute, values[vertexIndex]);
            }

            cnvVertex->SetIndex(attribute, newIndex);
        }

        primitive->mVertices.push_back(cnvVertex);
    }
}

void J3D::Cnv::UConverterVertexData::ProcessNBTData(const std::vector<glm::vec4>& tangents, const uint16_t vertexIndex, UConverterVertex* vertex) {
    glm::vec4 normal = mVertexData.at(EGXAttribute::Normal)[vertex->NormalIndex];
    glm::vec4 tangent = tangents[vertexIndex];

    glm::vec3 bitangent = glm::cross(
        glm::vec3(normal.x, normal.y, normal.z),
        glm::vec3(tangent.x, tangent.y, tangent.z)
    ) * tangent.w;

    UConverterNBTData nbt = { normal, tangent, bitangent };
    const auto itr = std::find(mNBTData.begin(), mNBTData.end(), nbt);

    if (itr == mNBTData.end()) {
        vertex->NormalIndex = mNBTData.size();
        mNBTData.push_back(nbt);

        return;
    }

    vertex->NormalIndex = itr - mNBTData.begin();
}

void J3D::Cnv::UConverterVertexData::WriteVTX1(bStream::CStream& stream) {
    size_t streamStartPos = stream.tell();

    stream.writeUInt32(0x56545831);
    stream.writeUInt32(0);
    stream.writeUInt32(0x40);

    // Offsets to optional attributes
    for (uint32_t i = 0; i < 13; i++) {
        stream.writeUInt32(0);
    }

    // Attribute storage definitions
    for (const auto& [attribute, values] : mVertexData) {
        uint32_t componentCount = 0, componentType = 0;
        uint8_t fixedPointExponent = 0;

        switch (attribute) {
            case EGXAttribute::Position:
                componentCount = static_cast<uint32_t>(EGXComponentCount::Position_XYZ);
                componentType = static_cast<uint32_t>(EGXComponentType::Float);
                fixedPointExponent = 0;
                break;
            case EGXAttribute::Normal:
                componentCount = static_cast<uint32_t>(EGXComponentCount::Normal_XYZ);
                componentType = static_cast<uint32_t>(EGXComponentType::Signed16);
                fixedPointExponent = FIXED_POINT_EXP_NORMAL;
                break;
            case EGXAttribute::Color0:
            case EGXAttribute::Color1:
                componentCount = static_cast<uint32_t>(EGXComponentCount::Color_RGBA);
                componentType = static_cast<uint32_t>(EGXComponentType::RGBA8);
                fixedPointExponent = 0;
                break;
            case EGXAttribute::TexCoord0:
            case EGXAttribute::TexCoord1:
            case EGXAttribute::TexCoord2:
            case EGXAttribute::TexCoord3:
            case EGXAttribute::TexCoord4:
            case EGXAttribute::TexCoord5:
            case EGXAttribute::TexCoord6:
            case EGXAttribute::TexCoord7:
                componentCount = static_cast<uint32_t>(EGXComponentCount::TexCoord_UV);
                componentType = static_cast<uint32_t>(EGXComponentType::Signed16);
                fixedPointExponent = FIXED_POINT_EXP_TEXCOORD;
                break;
            default:
                break;
        }

        stream.writeUInt32(static_cast<uint32_t>(attribute));
        stream.writeUInt32(componentCount);
        stream.writeUInt32(componentType);
        stream.writeUInt8(fixedPointExponent);

        stream.writeUInt8(UINT8_MAX);
        stream.writeUInt16(UINT16_MAX);
    }

    // Add null attribute
    stream.writeUInt32(static_cast<uint32_t>(EGXAttribute::Null));
    stream.writeUInt32(static_cast<uint32_t>(EGXComponentCount::Position_XYZ));
    stream.writeUInt32(0);
    stream.writeUInt8(0);

    stream.writeUInt8(UINT8_MAX);
    stream.writeUInt16(UINT16_MAX);

    // Pad attribute list to 16 bytes
    J3DUtility::PadStreamWithString(&stream, 16);

    // Attribute values
    for (const auto& [attribute, values] : mVertexData) {
        size_t currentStreamPos = stream.tell();

        uint32_t attributeOffset = 0;
        switch (attribute) {
            case EGXAttribute::Position:
            {
                attributeOffset = 0x0C;
                break;
            }
            case EGXAttribute::Normal:
            {
                attributeOffset = 0x10;
                break;
            }
            case EGXAttribute::NBT:
            {
                attributeOffset = 0x14;
                break;
            }
            case EGXAttribute::Color0:
            case EGXAttribute::Color1:
            {
                uint32_t colorIndex = static_cast<uint32_t>(attribute) - static_cast<uint32_t>(EGXAttribute::Color0);
                attributeOffset = 0x18 + colorIndex * 4;

                break;
            }
            case EGXAttribute::TexCoord0:
            case EGXAttribute::TexCoord1:
            case EGXAttribute::TexCoord2:
            case EGXAttribute::TexCoord3:
            case EGXAttribute::TexCoord4:
            case EGXAttribute::TexCoord5:
            case EGXAttribute::TexCoord6:
            case EGXAttribute::TexCoord7:
            {
                uint32_t texCoordIndex = static_cast<uint32_t>(attribute) - static_cast<uint32_t>(EGXAttribute::TexCoord0);
                attributeOffset = 0x20 + texCoordIndex * 4;

                break;
            }
            default:
            {
                break;
            }
        }

        stream.seek(streamStartPos + attributeOffset);
        stream.writeUInt32(static_cast<uint32_t>(currentStreamPos - streamStartPos));
        stream.seek(currentStreamPos);

        for (const glm::vec4& value : values) {
            switch (attribute) {
                case EGXAttribute::Position:
                    stream.writeFloat(value.x);
                    stream.writeFloat(value.y);
                    stream.writeFloat(value.z);

                    continue;
                case EGXAttribute::Normal:
                    stream.writeInt16(static_cast<int16_t>(value.x / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
                    stream.writeInt16(static_cast<int16_t>(value.y / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
                    stream.writeInt16(static_cast<int16_t>(value.z / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));

                    continue;
                case EGXAttribute::Color0:
                case EGXAttribute::Color1:
                    stream.writeUInt8(static_cast<uint8_t>(value.x * 255.0f));
                    stream.writeUInt8(static_cast<uint8_t>(value.y * 255.0f));
                    stream.writeUInt8(static_cast<uint8_t>(value.z * 255.0f));
                    stream.writeUInt8(static_cast<uint8_t>(value.w * 255.0f));

                    continue;
                case EGXAttribute::TexCoord0:
                case EGXAttribute::TexCoord1:
                case EGXAttribute::TexCoord2:
                case EGXAttribute::TexCoord3:
                case EGXAttribute::TexCoord4:
                case EGXAttribute::TexCoord5:
                case EGXAttribute::TexCoord6:
                case EGXAttribute::TexCoord7:
                    stream.writeInt16(static_cast<int16_t>(value.x / std::powf(0.5f, FIXED_POINT_EXP_TEXCOORD)));
                    stream.writeInt16(static_cast<int16_t>(value.y / std::powf(0.5f, FIXED_POINT_EXP_TEXCOORD)));

                    continue;
                default:
                    continue;
            }
        }

        // Pad section to 32 bytes
        J3DUtility::PadStreamWithString(&stream, 32);
    }

    // Write NBT data if present. Not handled above because it's ~*~special~*~
    if (mNBTData.size() != 0) {
        size_t currentStreamPos = stream.tell();

        // Write NBT offset. This is the only(?) way that J3D knows this data exists.
        // It's not technically listed in the attribute table.
        stream.seek(streamStartPos + 0x14);
        stream.writeUInt32(static_cast<uint32_t>(currentStreamPos - streamStartPos));
        stream.seek(currentStreamPos);

        WriteNBTData(stream);

        // Pad section to 32 bytes
        J3DUtility::PadStreamWithString(&stream, 32);
    }

    size_t streamEndPos = stream.tell();

    // Write section size
    stream.seek(streamStartPos + 4);
    stream.writeUInt32(static_cast<uint32_t>(streamEndPos - streamStartPos));
    stream.seek(streamEndPos);
}

void J3D::Cnv::UConverterVertexData::WriteNBTData(bStream::CStream& stream) {
    for (const auto& nbt : mNBTData) {
        stream.writeInt16(static_cast<int16_t>(nbt.Normal.x / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
        stream.writeInt16(static_cast<int16_t>(nbt.Normal.y / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
        stream.writeInt16(static_cast<int16_t>(nbt.Normal.z / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));

        stream.writeInt16(static_cast<int16_t>(nbt.Tangent.x / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
        stream.writeInt16(static_cast<int16_t>(nbt.Tangent.y / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
        stream.writeInt16(static_cast<int16_t>(nbt.Tangent.z / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));

        stream.writeInt16(static_cast<int16_t>(nbt.Bitangent.x / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
        stream.writeInt16(static_cast<int16_t>(nbt.Bitangent.y / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
        stream.writeInt16(static_cast<int16_t>(nbt.Bitangent.z / std::powf(0.5f, FIXED_POINT_EXP_NORMAL)));
    }

    // Pad section to 32 bytes
    J3DUtility::PadStreamWithString(&stream, 32);
}
