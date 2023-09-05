#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <string>

namespace tinygltf {
    class Model;
    class Node;
}

namespace J3D {
    namespace Cnv {
        class UConverterShape;

        struct UConverterJoint {
            std::string Name = "";

            // J3D properties
            uint16_t MatrixType = 1;
            bool bDoNotInheritParentScale = true;

            glm::vec3 Translation = glm::zero<glm::vec3>();
            glm::quat Rotation = glm::identity<glm::quat>();
            glm::vec3 Scale = glm::one<glm::vec3>();

            UConverterBoundingVolume Bounds;

            // Hierarchy properties
            UConverterJoint* Parent = nullptr;
            std::vector<UConverterJoint*> Children;

            // Utility properties
            uint32_t OriginalNodeIndex = UINT32_MAX;
            uint32_t JointIndex = UINT32_MAX;
            std::vector<UConverterShape*> AttachedShapes;

            void WriteHierarchyRecursive(bStream::CStream& stream);
        };

        class UConverterSkeletonData {
            std::vector<UConverterJoint*> mJoints;
            UConverterJoint* mRootJoint;

            void CreateDummyRoot(tinygltf::Model* model);

            void CreateSkeleton(tinygltf::Model* model);
            void BuildHierarchyRecursive(const std::vector<tinygltf::Node>& nodes, UConverterJoint* parent, uint32_t currentIndex);

        public:
            UConverterSkeletonData();
            ~UConverterSkeletonData();

            void BuildSkeleton(tinygltf::Model* model);

            void WriteINF1(bStream::CStream& stream, uint32_t vertexCount);
            void WriteJNT1(bStream::CStream& stream);

            void AttachShapeToJoint(UConverterShape* shape, uint32_t jointIndex) {
                if (jointIndex < mJoints.size()) {
                    mJoints[jointIndex]->AttachedShapes.push_back(shape);
                }
            }
        };
    }
}