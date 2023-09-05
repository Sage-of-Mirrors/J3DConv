#include "UConverterShapeData.hpp"
#include "UConverterVertexData.hpp"

#include <tiny_gltf.h>
#include <bstream.h>
#include <glm/ext.hpp>
#include <tri_stripper.h>

#include <algorithm>

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

/* UConverterVertex */

void J3D::Cnv::UConverterVertex::SetIndex(EGXAttribute attribute, uint16_t index) {
    switch (attribute) {
    case EGXAttribute::Position:
        PositionIndex = index;
        break;
    case EGXAttribute::Normal:
        NormalIndex = index;
        break;
    case EGXAttribute::Color0:
    case EGXAttribute::Color1:
    {
        uint32_t colorIdx = static_cast<uint32_t>(attribute) - static_cast<uint32_t>(EGXAttribute::Color0);
        ColorIndex[colorIdx] = index;
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
        uint32_t texIdx = static_cast<uint32_t>(attribute) - static_cast<uint32_t>(EGXAttribute::TexCoord0);
        TexCoordIndex[texIdx] = index;
        break;
    }
    default:
        break;
    }
}

/* UConverterPrimitive */

J3D::Cnv::UConverterPrimitive::~UConverterPrimitive() {
    for (UConverterVertex* v : mVertices) {
        delete v;
    }
    mVertices.clear();
}

/* UConverterShape */

J3D::Cnv::UConverterShape::UConverterShape() {

}

J3D::Cnv::UConverterShape::~UConverterShape() {
    for (UConverterPrimitive* p : mPrimitives) {
        delete p;
    }
    mPrimitives.clear();
}

/* UConverterShapeData */

J3D::Cnv::UConverterShapeData::UConverterShapeData() {

}

J3D::Cnv::UConverterShapeData::~UConverterShapeData() {
    for (UConverterShape* s : mShapes) {
        delete s;
    }
    mShapes.clear();
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

void J3D::Cnv::UConverterShapeData::ReadGltfVertexAttribute(
    const tinygltf::Model* model,
    std::vector<bStream::CMemoryStream>& buffers,
    uint32_t attributeAccessorIndex,
    std::vector<glm::vec4>& values
) {
    const auto& accessor = model->accessors[attributeAccessorIndex];
    const auto& view = model->bufferViews[accessor.bufferView];

    int32_t numComponents = tinygltf::GetNumComponentsInType(accessor.type);
    if (numComponents == -1) {
        return;
    }

    auto& attributeStream = buffers[view.buffer];
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

void J3D::Cnv::UConverterShapeData::ReadGltfIndices(
    const tinygltf::Model* model,
    std::vector<bStream::CMemoryStream>& buffers,
    uint32_t indexAccessorIndex,
    std::vector<uint16_t>& indices
) {
    const auto& indicesAccessor = model->accessors[indexAccessorIndex];
    const auto& indicesView = model->bufferViews[indicesAccessor.bufferView];

    auto& indicesStream = buffers[indicesView.buffer];
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

void J3D::Cnv::UConverterShapeData::BuildVertexData(tinygltf::Model* model, UConverterVertexData* vertexData, std::vector<bStream::CMemoryStream>& buffers) {
    for (const tinygltf::Mesh& mesh : model->meshes) {
        UConverterMesh* cnvMesh = new UConverterMesh();

        for (const tinygltf::Primitive& prim : mesh.primitives) {
            // Read vertex indices
            std::vector<uint16_t> rawIndices;
            ReadGltfIndices(model, buffers, prim.indices, rawIndices);

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

                    ReadGltfVertexAttribute(model, buffers, prim.attributes.at(name), primitiveAttributes[attribute]);
                }
                // Joint indices for skinning
                else if (name == "JOINTS_0") {
                    ReadGltfVertexAttribute(model, buffers, prim.attributes.at(name), jointIndices);
                }
                // Weights for skinning
                else if (name == "WEIGHTS_0") {
                    ReadGltfVertexAttribute(model, buffers, prim.attributes.at(name), weights);
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

                    vertexData->BuildConverterPrimitive(primitiveAttributes, strippedIndices, jointIndices, weights, cnvPrimitive);
                    cnvMesh->AddPrimitive(cnvPrimitive);
                }

                break;
            }
            // TODO: Add handling for other primitive types?
            default:
            {
                UConverterPrimitive* cnvPrimitive = new UConverterPrimitive();
                cnvPrimitive->mPrimitiveType = EGXPrimitiveType::TriangleStrips;

                vertexData->BuildConverterPrimitive(primitiveAttributes, rawIndices, jointIndices, weights, cnvPrimitive);
                cnvMesh->AddPrimitive(cnvPrimitive);

                break;
            }
            }
        }

        mMeshes.push_back(cnvMesh);
    }
}
