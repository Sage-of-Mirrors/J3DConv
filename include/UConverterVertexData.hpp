#pragma once

#include "types.hpp"

#include <glm/glm.hpp>

#include <map>
#include <vector>

namespace J3D {
    namespace Cnv {
        class UConverterPrimitive;
        struct UConverterVertex;

        struct UConverterNBTData {
            glm::vec3 Normal;
            glm::vec3 Tangent;
            glm::vec3 Bitangent;

            bool operator==(const UConverterNBTData& other) {
                if (Normal == other.Normal && Tangent == other.Tangent && Bitangent == other.Bitangent) {
                    return true;
                }

                return false;
            }

            bool operator!= (const UConverterNBTData& other) {
                return !(operator==(other));
            }
        };

        class UConverterVertexData {
            std::map<EGXAttribute, std::vector<glm::vec4>> mVertexData;
            std::vector<UConverterNBTData> mNBTData;

            void ProcessNBTData(const std::vector<glm::vec4>& tangents, const uint16_t vertexIndex, UConverterVertex* vertex);
            void WriteNBTData(bStream::CStream& stream);

        public:
            UConverterVertexData();
            ~UConverterVertexData();

            bool AttributeContainsValue(EGXAttribute attribute, const glm::vec4& value);
            uint16_t AddValueToAttribute(EGXAttribute attribute, const glm::vec4& value);
            uint16_t GetIndexOfValueInAttribute(EGXAttribute attribute, const glm::vec4& value);

            void BuildConverterPrimitive(const std::map<EGXAttribute, std::vector<glm::vec4>>& attributes,
                                         const std::vector<uint16_t>& indices,
                                         const std::vector<glm::vec4>& jointIndices,
                                         const std::vector<glm::vec4>& jointWeights,
                                         UConverterPrimitive* primitive);

            void WriteVTX1(bStream::CStream& stream);

            uint32_t GetVertexCount() const { return static_cast<uint32_t>(mVertexData.at(EGXAttribute::Position).size()); }
        };
    }
}
