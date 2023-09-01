#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <string>

namespace tinygltf {
    class Node;
}

namespace J3D {
    namespace Cnv {
        class UConverterJoint {
            std::string mName;
            
            glm::vec3 mPosition;
            glm::vec3 mScale;
            glm::quat mRotation;

            UConverterJoint* mParent;
            std::vector<UConverterJoint*> mChildren;

            uint32_t mOriginalIndex;

        public:
            UConverterJoint();
            UConverterJoint(UConverterJoint* parent);

            ~UConverterJoint();

            uint32_t GetOriginalIndex() const { return mOriginalIndex; }

            void GetJointsRecursive(const std::vector<tinygltf::Node>& nodes, std::vector<UConverterJoint*>& joints, uint32_t currentIndex);
        };
    }
}
