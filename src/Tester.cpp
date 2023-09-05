#include "J3DConv.hpp"
#include "J3DVertexData.hpp"
#include "UConverterObject.hpp"

#include <bstream.h>
#include <tiny_gltf.h>

int main() {
    tinygltf::Model model;

    J3D::Cnv::LoadGltf(&model, "D:\\SZS Tools\\J3DConv\\link.glb");

    J3D::Cnv::UConverterObject t;
    t.Load(&model);

    bStream::CFileStream f("D:\\SZS Tools\\J3DConv\\test.bmd", bStream::Big, bStream::Out);
    t.WriteBMD(f);
}