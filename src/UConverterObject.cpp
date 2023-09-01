#include "UConverterObject.hpp"

#include "J3DNameTable.hpp"
#include "J3DUtil.hpp"

#include <bstream.h>
#include <tiny_gltf.h>
#include <glm/gtx/quaternion.hpp>
#include <tri_stripper.h>

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

void J3D::Cnv::UConverterObject::WriteJNT1(bStream::CStream& stream, tinygltf::Model* model) {
    const tinygltf::Skin& skin = model->skins[0];
    const auto& nodes = model->nodes;

    size_t startingStreamPos = stream.tell();

    stream.writeUInt32(0x4A4E5431);
    stream.writeUInt32(0);

    stream.writeUInt16(skin.joints.size());
    stream.writeUInt16(UINT16_MAX);

    stream.writeUInt32(0);
    stream.writeUInt32(0);
    stream.writeUInt32(0);

    size_t jointTransformOffset = stream.tell();
    for (const int& jntIndex : skin.joints) {
        const tinygltf::Node& jntNode = nodes[jntIndex];

        stream.writeUInt16(0);
        stream.writeUInt8(0);
        stream.writeUInt8(UINT8_MAX);

        // Scale
        if (jntNode.scale.size() != 0) {
            stream.writeFloat(jntNode.scale[0]);
            stream.writeFloat(jntNode.scale[1]);
            stream.writeFloat(jntNode.scale[2]);
        }
        else {
            stream.writeFloat(1.0f);
            stream.writeFloat(1.0f);
            stream.writeFloat(1.0f);
        }

        // Rotation
        if (jntNode.rotation.size() != 0) {
            glm::quat rotationQuat(jntNode.rotation[3], jntNode.rotation[0], jntNode.rotation[1], jntNode.rotation[2]);
            glm::vec3 eulerAngles = glm::eulerAngles(rotationQuat);

            stream.writeInt16(glm::degrees(eulerAngles.x) * (32768.f / 180.f));
            stream.writeInt16(glm::degrees(eulerAngles.z) * (32768.f / 180.f));
            stream.writeInt16(glm::degrees(-eulerAngles.y) * (32768.f / 180.f));
        }
        else {
            stream.writeUInt16(0);
            stream.writeUInt16(0);
            stream.writeUInt16(0);
        }

        stream.writeUInt16(UINT16_MAX);

        // Translation
        if (jntNode.translation.size() != 0) {
            stream.writeFloat(jntNode.translation[0]);
            stream.writeFloat(jntNode.translation[1]);
            stream.writeFloat(jntNode.translation[2]);
        }
        else {
            stream.writeFloat(0.0f);
            stream.writeFloat(0.0f);
            stream.writeFloat(0.0f);
        }

        // Bounding sphere radius
        stream.writeFloat(0.0f);

        // Bounding box min
        stream.writeFloat(0.0f);
        stream.writeFloat(0.0f);
        stream.writeFloat(0.0f);

        // Bounding box max
        stream.writeFloat(0.0f);
        stream.writeFloat(0.0f);
        stream.writeFloat(0.0f);
    }

    size_t instanceTableOffset = stream.tell();
    for (uint32_t i = 0; i < skin.joints.size(); i++) {
        stream.writeUInt16(i);
    }

    J3DUtility::PadStreamWithString(&stream, 4);
    size_t nameTableOffset = stream.tell();

    J3DNameTable nameTable;
    for (const int& jntIndex : skin.joints) {
        const tinygltf::Node& jntNode = nodes[jntIndex];

        nameTable.AddName(jntNode.name);
    }

    nameTable.Serialize(&stream);

    size_t endingStreamPos = stream.tell();

    stream.seek(startingStreamPos + 0xC);
    stream.writeUInt32(jointTransformOffset - startingStreamPos);
    stream.writeUInt32(instanceTableOffset - startingStreamPos);
    stream.writeUInt32(nameTableOffset - startingStreamPos);

    stream.seek(startingStreamPos + 0x4);
    stream.writeUInt32(endingStreamPos - startingStreamPos);

    stream.seek(endingStreamPos);
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

J3D::Cnv::EGXAttribute GetVertexAttributeFromType(const std::string& type) {
    if (type == "POSITION") {
        return J3D::Cnv::EGXAttribute::Position;
    }
    else if (type == "NORMAL") {
        return J3D::Cnv::EGXAttribute::Normal;
    }
    else if (type == "TANGENT") {
        return J3D::Cnv::EGXAttribute::NBT;
    }
    else if (type == "COLOR_0") {
        return J3D::Cnv::EGXAttribute::Color0;
    }
    else if (type == "COLOR_1") {
        return J3D::Cnv::EGXAttribute::Color1;
    }
    else if (type == "TEXCOORD_0") {
        return J3D::Cnv::EGXAttribute::TexCoord0;
    }
    else if (type == "TEXCOORD_1") {
        return J3D::Cnv::EGXAttribute::TexCoord1;
    }
    else if (type == "TEXCOORD_2") {
        return J3D::Cnv::EGXAttribute::TexCoord2;
    }
    else if (type == "TEXCOORD_3") {
        return J3D::Cnv::EGXAttribute::TexCoord3;
    }
    else if (type == "TEXCOORD_4") {
        return J3D::Cnv::EGXAttribute::TexCoord4;
    }
    else if (type == "TEXCOORD_5") {
        return J3D::Cnv::EGXAttribute::TexCoord5;
    }
    else if (type == "TEXCOORD_6") {
        return J3D::Cnv::EGXAttribute::TexCoord6;
    }
    else if (type == "TEXCOORD_7") {
        return J3D::Cnv::EGXAttribute::TexCoord7;
    }
    
    return J3D::Cnv::EGXAttribute::Null;
}

const std::vector<std::string> VERTEX_ATTRIBUTE_NAMES = {
    "POSITION",
    "NORMAL",
    "TANGENT",
    "COLOR_0",
    "COLOR_1",
    "TEXCOORD_0",
    "TEXCOORD_1",
    "TEXCOORD_2",
    "TEXCOORD_3",
    "TEXCOORD_4",
    "TEXCOORD_5",
    "TEXCOORD_6",
    "TEXCOORD_7",
};

void J3D::Cnv::UConverterObject::ReadGltfVertexAttribute(const tinygltf::Model* model, uint32_t attributeAccessorIndex, std::vector<glm::vec4>& values) {
    const auto& accessor = model->accessors[attributeAccessorIndex];
    const auto& view = model->bufferViews[accessor.bufferView];

    int32_t numComponents = tinygltf::GetNumComponentsInType(accessor.type);
    if (numComponents == -1) {
        return;
    }

    auto& attributeStream = mBufferStreams[view.buffer];
    attributeStream.seek(view.byteOffset);

    for (uint32_t i = 0; i < accessor.count; i++) {
        glm::vec4 value = glm::zero<glm::vec4>();

        for (uint32_t j = 0; j < numComponents; j++) {
            float compValue = 0;

            switch (accessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_BYTE:
                compValue = static_cast<float>(attributeStream.readInt8());
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                compValue = static_cast<float>(attributeStream.readUInt8());
                break;
            case TINYGLTF_COMPONENT_TYPE_SHORT:
                compValue = static_cast<float>(attributeStream.readInt16());
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                compValue = static_cast<float>(attributeStream.readUInt16());
                break;
            case TINYGLTF_COMPONENT_TYPE_INT:
                compValue = static_cast<float>(attributeStream.readInt32());
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                compValue = static_cast<float>(attributeStream.readUInt32());
                break;
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                compValue = static_cast<float>(attributeStream.readFloat());
                break;
            default:
                break;
            }

            value[j] = compValue;
        }

        values.push_back(value);
    }
}

void J3D::Cnv::UConverterObject::ReadGltfIndices(const tinygltf::Model* model, uint32_t indexAccessorIndex, std::vector<uint16_t>& indices) {
    const auto& indicesAccessor = model->accessors[indexAccessorIndex];
    const auto& indicesView = model->bufferViews[indicesAccessor.bufferView];

    auto& indicesStream = mBufferStreams[indicesView.buffer];
    indicesStream.seek(indicesView.byteOffset);

    for (uint32_t i = 0; i < indicesAccessor.count; i++) {
        uint16_t index = UINT32_MAX;

        switch (indicesAccessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                index = static_cast<uint16_t>(indicesStream.readUInt8());
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                index = static_cast<uint16_t>(indicesStream.readUInt16());
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                index = static_cast<uint16_t>(indicesStream.readUInt32());
                break;
            default:
                break;
        }

        indices.push_back(index);
    }
}

void J3D::Cnv::UConverterObject::BuildVertexData(tinygltf::Model* model) {
    const auto& accessors = model->accessors;
    const auto& views = model->bufferViews;

    for (const tinygltf::Mesh& mesh : model->meshes) {
        UConverterMesh* cnvMesh = new UConverterMesh();

        for (const tinygltf::Primitive& prim : mesh.primitives) {
            // Read vertex indices
            std::vector<uint16_t> rawIndices;
            ReadGltfIndices(model, prim.indices, rawIndices);

            // Read vertex attributes
            std::map<EGXAttribute, std::vector<glm::vec4>> primitiveAttributes;
            std::vector<glm::vec4> jointIndices;
            std::vector<glm::vec4> weights;

            for (const auto& [name, index] : prim.attributes) {
                // Vertex attributes
                if (std::find(VERTEX_ATTRIBUTE_NAMES.begin(), VERTEX_ATTRIBUTE_NAMES.end(), name) != VERTEX_ATTRIBUTE_NAMES.end()) {
                    EGXAttribute attribute = GetVertexAttributeFromType(name);
                    if (attribute == EGXAttribute::Null) {
                        continue;
                    }

                    ReadGltfVertexAttribute(model, prim.attributes.at(name), primitiveAttributes[attribute]);
                }
                // Joint indices for skinning
                else if (name == "JOINTS_0") {
                    ReadGltfVertexAttribute(model, prim.attributes.at(name), jointIndices);
                }
                // Weights for skinning
                else if (name == "WEIGHTS_0") {
                    ReadGltfVertexAttribute(model, prim.attributes.at(name), weights);
                }
                else {
                    std::cout << "Unknown glTF attribute \'" << name << "\'!" << std::endl;
                }
            }

            // Process index data and add vertex attributes to the vertex data arrays
            switch (prim.mode) {
                case TINYGLTF_MODE_TRIANGLES:
                {
                    triangle_stripper::indices indicesToStrip;
                    for (const uint16_t i : rawIndices) {
                        indicesToStrip.push_back(static_cast<size_t>(i));
                    }

                    triangle_stripper::tri_stripper stripper(indicesToStrip);

                    triangle_stripper::primitive_vector strippedPrimitives;
                    stripper.Strip(&strippedPrimitives);

                    for (const auto& strip : strippedPrimitives) {
                        UConverterPrimitive* cnvPrimitive = new UConverterPrimitive();
                        cnvPrimitive->mPrimitiveType = EGXPrimitiveType::TriangleStrips;

                        std::vector<uint16_t> strippedIndices;
                        for (const triangle_stripper::index i : strip.Indices) {
                            strippedIndices.push_back(static_cast<uint16_t>(i));
                        }

                        mVertexData.BuildConverterPrimitive(primitiveAttributes, strippedIndices, jointIndices, weights, cnvPrimitive);
                        cnvMesh->AddPrimitive(cnvPrimitive);
                    }

                    break;
                }
                // TODO: Add handling for other primitive types?
                default:
                {
                    UConverterPrimitive* cnvPrimitive = new UConverterPrimitive();
                    cnvPrimitive->mPrimitiveType = EGXPrimitiveType::TriangleStrips;

                    mVertexData.BuildConverterPrimitive(primitiveAttributes, rawIndices, jointIndices, weights, cnvPrimitive);
                    cnvMesh->AddPrimitive(cnvPrimitive);

                    break;
                }
            }
        }

        mMeshes.push_back(cnvMesh);
    }
}

bool J3D::Cnv::UConverterObject::Load(tinygltf::Model* model) {
    LoadBuffers(model);
    BuildVertexData(model);

    uint32_t sceneRootNodeIndex = model->scenes[model->defaultScene].nodes[0];
    auto& sceneRootNode = model->nodes[sceneRootNodeIndex];

    bStream::CFileStream f("D:\\SZS Tools\\J3DConv\\test.bmd", bStream::Big, bStream::Out);

    f.writeUInt32(0x4A334432);
    f.writeUInt32(0x626D6433);
    f.writeUInt32(0);
    f.writeUInt32(8);

    for (uint32_t i = 0; i < 0x10; i++) {
        f.writeUInt8(UINT8_MAX);
    }

    //WriteINF1();
    mVertexData.WriteVTX1(f);
    //WriteEVP1();
    //WriteDRW1();

    //WriteJNT1(f, model);
    
    //WriteSHP1();
    //WriteMAT3();
    
    WriteTEX1(f, model);

    size_t fileSize = f.tell();

    f.seek(8);
    f.writeUInt32(fileSize);

    return true;
}