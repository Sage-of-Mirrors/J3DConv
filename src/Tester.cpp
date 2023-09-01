#include "J3DConv.hpp"
#include "J3DVertexData.hpp"
#include "UConverterObject.hpp"

#include <tiny_gltf.h>

#include <iostream>

int main() {
    tinygltf::Model model;

    J3D::Cnv::LoadGltf(&model, "D:\\SZS Tools\\J3DConv\\testspheres.glb");

    J3D::Cnv::UConverterObject t;
    t.Load(&model);

    //J3D::Cnv::SaveBMD(&model, "D:\\SZS Tools\\J3DConv\\testman_skinned.bmd");
}