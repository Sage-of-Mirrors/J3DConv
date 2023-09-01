#include "UConverterMesh.hpp"

#include <bstream.h>
#include <tiny_gltf.h>

void J3D::Cnv::UConverterVertex::SetIndex(EGXAttribute attribute, uint16_t index) {
    switch (attribute) {
        case EGXAttribute::Position:
            PositionIndex = index;
            break;
        case EGXAttribute::Normal:
            NormalIndex = index;
            break;
        case EGXAttribute::Color0:
        case EGXAttribute::Color1:
        {
            uint32_t colorIdx = static_cast<uint32_t>(attribute) - static_cast<uint32_t>(EGXAttribute::Color0);
            ColorIndex[colorIdx] = index;
            break;
        }
        case EGXAttribute::TexCoord0:
        case EGXAttribute::TexCoord1:
        case EGXAttribute::TexCoord2:
        case EGXAttribute::TexCoord3:
        case EGXAttribute::TexCoord4:
        case EGXAttribute::TexCoord5:
        case EGXAttribute::TexCoord6:
        case EGXAttribute::TexCoord7:
        {
            uint32_t texIdx = static_cast<uint32_t>(attribute) - static_cast<uint32_t>(EGXAttribute::TexCoord0);
            TexCoordIndex[texIdx] = index;
            break;
        }
        default:
            break;
    }
}

J3D::Cnv::UConverterPrimitive::~UConverterPrimitive() {
    for (UConverterVertex* v : mVertices) {
        delete v;
    }
    mVertices.clear();
}

J3D::Cnv::UConverterMesh::UConverterMesh() {

}

J3D::Cnv::UConverterMesh::~UConverterMesh() {
    for (UConverterPrimitive* prim : mPrimitives) {
        delete prim;
    }
    mPrimitives.clear();
}
