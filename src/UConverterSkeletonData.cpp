#include "UConverterSkeletonData.hpp"
#include "J3DNameTable.hpp"
#include "J3DUtil.hpp"

#include <bstream.h>
#include <tiny_gltf.h>

const float INT16_RAD_ANGLE_RATIO = 32768.0f / glm::pi<float>();

/* UConverterJoint */

void J3D::Cnv::UConverterJoint::WriteHierarchyRecursive(bStream::CStream& stream) {
    stream.writeUInt16(0x10);
    stream.writeUInt16(JointIndex);

    for (UConverterJoint* j : Children) {
        stream.writeUInt16(1);
        stream.writeUInt16(0);

        j->WriteHierarchyRecursive(stream);

        stream.writeUInt16(2);
        stream.writeUInt16(0);
    }
}

/* UConverterSkeletonData */

J3D::Cnv::UConverterSkeletonData::UConverterSkeletonData() : mRootJoint(nullptr) {

}

J3D::Cnv::UConverterSkeletonData::~UConverterSkeletonData() {
    mRootJoint = nullptr;

    for (UConverterJoint* j : mJoints) {
        delete j;
    }

    mJoints.clear();
}

void J3D::Cnv::UConverterSkeletonData::CreateDummyRoot(tinygltf::Model* model) {
    std::string rootName = "root";

    // Not sure if these checks are required, but I'll do them anyway.
    // If we can't get the name of the object for the root joint name,
    // it will just default to "root".
    if (model->scenes.size() != 0 && model->nodes.size() != 0) {
        const auto& scene = model->scenes[model->defaultScene];
        const auto& node = model->nodes[scene.nodes[0]];

        rootName = node.name;
    }

    UConverterJoint* dummyRoot = new UConverterJoint();
    dummyRoot->Name = rootName;

    mJoints.push_back(dummyRoot);
    mRootJoint = dummyRoot;
}

void J3D::Cnv::UConverterSkeletonData::CreateSkeleton(tinygltf::Model* model) {
    if (model->scenes.size() == 0 || model->nodes.size() == 0 || model->skins.size() == 0) {
        return;
    }

    const auto& nodes = model->nodes;
    const auto& skin = model->skins[0];

    uint32_t sceneRootNodeIndex = model->scenes[model->defaultScene].nodes[0];
    auto& sceneRootNode = nodes[sceneRootNodeIndex];

    uint32_t rootJointNodeIndex = UINT32_MAX;

    // First, create all the joints at once to ensure
    // that they are in the correct order.
    for (const int jointIndex : skin.joints) {
        const auto& currentNode = nodes[jointIndex];

        UConverterJoint* cnvJoint = new UConverterJoint();
        cnvJoint->Name = currentNode.name;
        cnvJoint->OriginalNodeIndex = jointIndex;
        cnvJoint->JointIndex = mJoints.size();

        if (currentNode.scale.size() != 0) {
            cnvJoint->Scale = {
                static_cast<float>(currentNode.scale[0]),
                static_cast<float>(currentNode.scale[1]),
                static_cast<float>(currentNode.scale[2])
            };
        }

        if (currentNode.translation.size() != 0) {
            cnvJoint->Translation = {
                static_cast<float>(currentNode.translation[0]),
                static_cast<float>(currentNode.translation[1]),
                static_cast<float>(currentNode.translation[2])
            };
        }

        if (currentNode.rotation.size() != 0) {
            cnvJoint->Rotation = {
                static_cast<float>(currentNode.rotation[3]),
                static_cast<float>(currentNode.rotation[0]),
                static_cast<float>(currentNode.rotation[1]),
                static_cast<float>(currentNode.rotation[2])
            };
        }

        mJoints.push_back(cnvJoint);

        // Find the index of the skeleton's root node while we're at it.
        if (rootJointNodeIndex == UINT32_MAX) {
            for (const int i : sceneRootNode.children) {
                if (i == jointIndex) {
                    rootJointNodeIndex = i;
                    break;
                }
            }
        }
    }

    // Once we've created all the nodes, bind them together in a hierarchy.
    BuildHierarchyRecursive(nodes, nullptr, rootJointNodeIndex);
}

void J3D::Cnv::UConverterSkeletonData::BuildHierarchyRecursive(const std::vector<tinygltf::Node>& nodes, UConverterJoint* parent, uint32_t currentIndex) {
    auto currentJointItr = std::find_if(
        mJoints.begin(),
        mJoints.end(),
        [&currentIndex](UConverterJoint* j) {
            return j->OriginalNodeIndex == currentIndex;
        }
    );

    // Don't know if this is possible, but just in case...
    if (currentJointItr == mJoints.end()) {
        return;
    }

    auto* currentJoint = *currentJointItr;

    if (parent == nullptr) {
        mRootJoint = currentJoint;
    }
    else {
        currentJoint->Parent = parent;
        parent->Children.push_back(currentJoint);
    }

    const auto& currentNode = nodes[currentIndex];
    for (const auto i : currentNode.children) {
        BuildHierarchyRecursive(nodes, currentJoint, i);
    }
}

void J3D::Cnv::UConverterSkeletonData::BuildSkeleton(tinygltf::Model* model) {
    if (model == nullptr) {
        return;
    }

    if (model->skins.size() == 0) {
        CreateDummyRoot(model);
    }
    else {
        CreateSkeleton(model);
    }
}

void J3D::Cnv::UConverterSkeletonData::WriteINF1(bStream::CStream& stream, uint32_t vertexCount) {
    size_t streamStartPos = stream.tell();

    // Write header
    stream.writeUInt32(0x494E4631); // FourCC ('INF1')
    stream.writeUInt32(0);          // Placeholder for section size
    stream.writeUInt16(0);          // "Misc flags"?
    stream.writeUInt16(UINT16_MAX); // Padding

    stream.writeUInt32(0);           // Matrix group count
    stream.writeUInt32(vertexCount); // Vertex count
    stream.writeUInt32(0x18);        // Offset to hierarchy data; always 0x18

    // Write hierarchy
    mRootJoint->WriteHierarchyRecursive(stream);

    // End the hierarchy
    stream.writeUInt32(0);

    J3DUtility::PadStreamWithString(&stream, 32);

    // Write section size
    J3DUtility::WriteOffset(&stream, streamStartPos, 0x04);
}

void J3D::Cnv::UConverterSkeletonData::WriteJNT1(bStream::CStream& stream) {
    J3DNameTable jointNameTable;
    size_t streamStartPos = stream.tell();

    // Header
    stream.writeUInt32(0x4A4E5431);     // FourCC ('JNT1')
    stream.writeUInt32(0);              // Placeholder for section size
    stream.writeUInt16(mJoints.size()); // Number of joints
    stream.writeUInt16(UINT16_MAX);     // Padding

    // Offsets
    stream.writeUInt32(0); // Placeholder for joint data offset
    stream.writeUInt32(0); // Placeholder for instance table offset
    stream.writeUInt32(0); // Placeholder for name table offset

    // Write joint data offset
    J3DUtility::WriteOffset(&stream, streamStartPos, 0x0C);
    // Write joint data
    for (const UConverterJoint* j : mJoints) {
        jointNameTable.AddName(j->Name);

        // Header
        stream.writeUInt16(j->MatrixType);
        stream.writeUInt8(j->bDoNotInheritParentScale);
        stream.writeUInt8(UINT8_MAX);

        // Scale
        stream.writeFloat(j->Scale.x);
        stream.writeFloat(j->Scale.y);
        stream.writeFloat(j->Scale.z);

        // Rotation
        glm::vec3 eulerAngles = glm::eulerAngles(j->Rotation);
        stream.writeInt16(eulerAngles.x * INT16_RAD_ANGLE_RATIO);
        stream.writeInt16(eulerAngles.y * INT16_RAD_ANGLE_RATIO);
        stream.writeInt16(eulerAngles.z * INT16_RAD_ANGLE_RATIO);
        stream.writeUInt16(UINT16_MAX);

        // Translation
        stream.writeFloat(j->Translation.x);
        stream.writeFloat(j->Translation.y);
        stream.writeFloat(j->Translation.z);

        // Bounding sphere radius
        stream.writeFloat(j->Bounds.BoundingSphereRadius);

        // Bounding box min
        stream.writeFloat(j->Bounds.BoundingBoxMin.x);
        stream.writeFloat(j->Bounds.BoundingBoxMin.y);
        stream.writeFloat(j->Bounds.BoundingBoxMin.z);

        // Bounding box max
        stream.writeFloat(j->Bounds.BoundingBoxMax.x);
        stream.writeFloat(j->Bounds.BoundingBoxMax.y);
        stream.writeFloat(j->Bounds.BoundingBoxMax.z);
    }

    // Write instance table offset
    J3DUtility::WriteOffset(&stream, streamStartPos, 0x10);
    // Write instance table
    for (uint16_t i = 0; i < mJoints.size(); i++) {
        stream.writeUInt16(i);
    }

    J3DUtility::PadStreamWithString(&stream, 4);

    // Write name table offset
    J3DUtility::WriteOffset(&stream, streamStartPos, 0x14);
    // Write name table
    jointNameTable.Serialize(&stream);

    J3DUtility::PadStreamWithString(&stream, 32);

    // Write section size
    J3DUtility::WriteOffset(&stream, streamStartPos, 0x04);
}