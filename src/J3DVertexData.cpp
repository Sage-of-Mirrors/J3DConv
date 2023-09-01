#include "J3DVertexData.hpp"

#include "tiny_gltf.h"
#include <bstream.h>

J3D::Cnv::J3DVertexData::J3DVertexData() {

}

const uint8_t* GetBufferAccessor(const tinygltf::Model* model, size_t accessorIdx, size_t& size) {
    const auto& accessor = model->accessors.at(accessorIdx);
    const auto& bufferView = model->bufferViews[accessor.bufferView];

    size = bufferView.byteLength;
    return &model->buffers[0].data[0] + bufferView.byteOffset;
}

J3D::Cnv::EGXAttribute GetGXAttribute(const std::string attributeName) {
    if (attributeName == "POSITION") {
        return J3D::Cnv::EGXAttribute::Position;
    }
    else if (attributeName == "NORMAL") {
        return J3D::Cnv::EGXAttribute::Normal;
    }
    else {
        return J3D::Cnv::EGXAttribute::Null;
    }
}

bool J3D::Cnv::J3DVertexData::LoadAttributeData(const tinygltf::Model* model) {
    if (model == nullptr) {
        return false;
    }

    for (auto& mesh : model->meshes) {
        for (auto& prim : mesh.primitives) {
            for ( auto const& [name, accessorIdx] : prim.attributes) {
                EGXAttribute curAttribute = GetGXAttribute(name);
                if (curAttribute != EGXAttribute::Null) {
                    AttributeData[curAttribute] = std::vector<glm::vec4>();
                }

                const tinygltf::Accessor& accessor = model->accessors.at(accessorIdx);
                const tinygltf::BufferView& bufferView = model->bufferViews[accessor.bufferView];

                size_t bufferSize = bufferView.byteLength;
                uint8_t* buf = new uint8_t[bufferSize]{};
                std::memcpy(buf, model->buffers[bufferView.buffer].data.data() + bufferView.byteOffset, bufferSize);

                bStream::CMemoryStream stream(buf, bufferSize, bStream::Little, bStream::In);

                for (int i = 0; i < accessor.count; i++) {
                    switch (accessor.type) {
                        case TINYGLTF_TYPE_VEC3:
                        {
                            switch (accessor.componentType) {
                                case TINYGLTF_COMPONENT_TYPE_FLOAT:
                                {
                                    AttributeData[curAttribute].push_back(glm::vec4(stream.readFloat(), stream.readFloat(), stream.readFloat(), 0.0f));
                                    break;
                                }
                            break;
                            }
                        }
                    }
                }

                delete[] buf;
            }
        }
    }

    return true;
}

bool J3D::Cnv::J3DVertexData::SaveVtx1(bStream::CStream& stream) {
    return true;
}