#ifndef SPIRE_SHADER_GREEDY
#define SPIRE_SHADER_GREEDY

#include "ShaderInfo.h"

struct MeshFace {
    float Width;
    float Height;
    SPIRE_UINT32_TYPE SizeX;
    SPIRE_UINT32_TYPE SizeY;
    SPIRE_UINT32_TYPE SizeZ;
    SPIRE_UINT32_TYPE VoxelTypesStartIndex;
};

#endif