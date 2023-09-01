#include "UConverterJoint.hpp"

#include <glm/gtc/constants.hpp>
#include <bstream.h>
#include <tiny_gltf.h>

J3D::Cnv::UConverterJoint::UConverterJoint() : UConverterJoint(nullptr)  {

}

J3D::Cnv::UConverterJoint::UConverterJoint(UConverterJoint* parent) : mParent(parent), mPosition(glm::zero<glm::vec3>()),
    mScale(glm::one<glm::vec3>()), mRotation(glm::identity<glm::quat>()), mOriginalIndex(UINT32_MAX) {

}

J3D::Cnv::UConverterJoint::~UConverterJoint() {

}

void J3D::Cnv::UConverterJoint::GetJointsRecursive(const std::vector<tinygltf::Node>& nodes, std::vector<UConverterJoint*>& joints, uint32_t currentIndex) {
    joints.push_back(this);

    mOriginalIndex = currentIndex;
    const tinygltf::Node& currentNode = nodes[currentIndex];

    mName = currentNode.name;

    if (currentNode.translation.size() != 0) {
        mPosition = glm::vec3(currentNode.translation[0], currentNode.translation[1], currentNode.translation[2]);
    }

    if (currentNode.scale.size() != 0) {
        mScale = glm::vec3(currentNode.scale[0], currentNode.scale[1], currentNode.scale[2]);
    }

    if (currentNode.rotation.size() != 0) {
        mRotation = glm::quat(currentNode.rotation[3], currentNode.rotation[0], currentNode.rotation[1], currentNode.rotation[2]);
    }

    for (uint32_t i = 0; i < currentNode.children.size(); i++) {
        UConverterJoint* child = new UConverterJoint(this);
        mChildren.push_back(child);

        child->GetJointsRecursive(nodes, joints, currentNode.children[i]);
    }
}
