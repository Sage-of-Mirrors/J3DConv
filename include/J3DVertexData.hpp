#pragma once

#include "types.hpp"

#include <glm/glm.hpp>

#include <map>
#include <vector>

namespace J3D {
    namespace Cnv {
        class J3DVertexData {
            std::map<EGXAttribute, std::vector<glm::vec4>> AttributeData;

        public:
            J3DVertexData();

            bool LoadAttributeData(const tinygltf::Model* model);
            bool SaveVtx1(bStream::CStream& stream);
        };
    }
}
