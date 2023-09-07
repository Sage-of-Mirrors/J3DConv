#pragma once;

#include <glm/vec3.hpp>
#include <cstdint>

namespace bStream {
    class CStream;
}

namespace tinygltf {
    class Model;
}

namespace J3D {
    namespace Cnv {
        // Represents a per-vertex attribute sent to the GPU.
        enum class EGXAttribute : uint8_t {
            PositionMatrixIdx = 0,
            Tex0MatrixIdx,
            Tex1MatrixIdx,
            Tex2MatrixIdx,
            Tex3MatrixIdx,
            Tex4MatrixIdx,
            Tex5MatrixIdx,
            Tex6MatrixIdx,
            Tex7MatrixIdx,

            Position,
            Normal,
            Color0,
            Color1,
            TexCoord0,
            TexCoord1,
            TexCoord2,
            TexCoord3,
            TexCoord4,
            TexCoord5,
            TexCoord6,
            TexCoord7,

            PositionMatrixArray,
            NormalMatrixArray,
            TexMatrixArray,
            LightArray,
            NBT,

            Attribute_Max,
            Null = 0xFF
        };

        // Represents what components a per-vertex attribute has, depending on the attribute.
        enum class EGXComponentCount : uint8_t {
            Position_XY = 0,
            Position_XYZ,

            Normal_XYZ = 0,
            Normal_NBT,
            Normal_NBT3,

            Color_RGB = 0,
            Color_RGBA,

            TexCoord_U = 0,
            TexCoord_UV
        };

        // Represents the underlying type of a per-vertex attribute.
        enum class EGXComponentType : uint8_t {
            Unsigned8 = 0,
            Signed8,
            Unsigned16,
            Signed16,
            Float,

            // Colors
            RGB565 = 0,
            RGB8,
            RGBX8,
            RGBA4,
            RGBA6,
            RGBA8
        };

        // Represents what type of index a per-vertex attribute has within a primitive.
        enum class EGXAttributeIndexType : uint8_t {
            None = 0,
            Direct,
            Index8,
            Index16
        };

        // Represents what primitive a list of vertices forms.
        enum class EGXPrimitiveType : uint8_t {
            None = 0,

            Quads = 0x80,
            Triangles = 0x90,
            TriangleStrips = 0x98,
            TriangleFan = 0xA0,
            Lines = 0xA8,
            LineStrips = 0xB0,
            Points = 0xB8
        };

        struct UConverterBoundingVolume {
            float BoundingSphereRadius = 0.0f;

            glm::vec3 BoundingBoxMin = { FLT_MAX, FLT_MAX, FLT_MAX };
            glm::vec3 BoundingBoxMax = { FLT_MIN, FLT_MIN, FLT_MIN };
        };
    }
}