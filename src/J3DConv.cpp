#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "J3DConv.hpp"

#include <bstream.h>
#include <tiny_gltf.h>

#include <iostream>

bool J3D::Cnv::LoadGltf(tinygltf::Model* model, std::filesystem::path filePath) {
    if (filePath.empty() || !std::filesystem::exists(filePath)) {
        return false;
    }

    bStream::CFileStream stream(filePath.string().c_str(), bStream::Little, bStream::In);

    size_t bufSize = stream.getSize();
    uint8_t* buf = new uint8_t[bufSize];

    if (buf == nullptr) {
        return false;
    }

    stream.readBytesTo(buf, bufSize);
    bool result = LoadGltf(model, buf, bufSize);

    delete[] buf;
    return result;
}

bool J3D::Cnv::LoadGltf(tinygltf::Model* model, const uint8_t* data, size_t size) {
    tinygltf::TinyGLTF loader;
    std::string error = "";
    std::string warning = "";

    bool result = loader.LoadBinaryFromMemory(model, &error, &warning, data, (unsigned int)size);

    if (!error.empty()) {
        std::cout << "glTF loader emitted errors: " << error << std::endl;
    }

    if (!warning.empty()) {
        std::cout << "glTF loader emitted warnings: " << warning << std::endl;
    }

    return result;
}

bool J3D::Cnv::SaveBMD(tinygltf::Model* model, std::filesystem::path filePath) {
    if (model == nullptr || filePath.empty()) {
        return false;
    }

    bStream::CFileStream stream(filePath.string().c_str(), bStream::Big, bStream::Out);
    return SaveBMD(model, stream);
}

bool J3D::Cnv::SaveBMD(tinygltf::Model* model, bStream::CStream& stream) {
    if (model == nullptr) {
        return false;
    }

    return true;
}
