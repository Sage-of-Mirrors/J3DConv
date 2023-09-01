#pragma once

#include "types.hpp"

#include <filesystem>
#include <cstdint>

namespace J3D {
    namespace Cnv {
        bool LoadGltf(tinygltf::Model* model, std::filesystem::path filePath);
        bool LoadGltf(tinygltf::Model* model, const uint8_t* data, size_t size);

        bool SaveBMD(tinygltf::Model* model, std::filesystem::path filePath);
        bool SaveBMD(tinygltf::Model* model, bStream::CStream& stream);
    }
}
