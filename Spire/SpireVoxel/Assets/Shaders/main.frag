#version 460
#extension GL_EXT_nonuniform_qualifier : require

#include "ShaderInfo.h"
#include "PushConstants.h"
#include "Greedy.h"

// See vertex shader for input documentation
layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 voxelData;
layout (location = 2) flat in uint voxelDataChunkIndex;
layout (location = 3) flat in uint voxelFace;
layout (location = 4) flat in uint voxelDataAllocationIndex;
layout (location = 5) flat in uint voxelTypesFaceStartIndex;
layout (location = 6) flat in float faceWidth;
layout (location = 7) flat in float faceHeight;

layout(location = 0) out vec4 out_Color;

// All the textures used by voxels
layout(set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING) uniform sampler2D texSampler[];

// The types of all the voxels in the current world
layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_VOXEL_DATA_BINDING) readonly buffer ChunkVoxelData {
    uint datas[];
} chunkVoxelData[];

// Information about all registered voxel types
layout (set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING) readonly buffer VoxelTypesBuffer {
    GPUVoxelType voxelTypes[];
} voxelTypesBuffer;

#define NUM_TYPES_PER_INT 2u// 2 voxel types (u16) in a u32

uint roundToUint(float x) {
    return uint(floor(x));
}

void main() {
    // Get voxel type
    uvec2 voxelCoordsInFace = uvec2(uv);
    if (IsFaceOnYAxis(voxelFace)) {
      //  voxelCoordsInFace.yx = voxelCoordsInFace.xy;
    }

    if (IsFaceOnZAxis(voxelFace)) {
     //   voxelCoordsInFace.yx = voxelCoordsInFace.xy;
        if (voxelFace ==SPIRE_VOXEL_FACE_NEG_Z) {
      //   voxelCoordsInFace.y = (uint(faceHeight) - 1) - voxelCoordsInFace.y;
        }
    }
    if ( voxelFace == SPIRE_VOXEL_FACE_NEG_Z) {
        // need to flip x coords
        voxelCoordsInFace.x = (uint(faceWidth) - 1) - voxelCoordsInFace.x;
    }

    if (voxelFace == SPIRE_VOXEL_FACE_NEG_Y) {
        // need to flip y coords
        //  voxelCoordsInFace.y = (uint(faceHeight) - 1) - voxelCoordsInFace.y;
    }

    uint voxelIndexInFace = uint(voxelCoordsInFace.y * faceWidth + voxelCoordsInFace.x);

    uint voxelDataIndex = voxelIndexInFace + voxelTypesFaceStartIndex;

    vec2 debugUV = vec2(voxelCoordsInFace);
    if (uv.x!=-239) {
        out_Color = vec4(debugUV.y / 64, 0, 0, 1.0);
        // return;
    }


    //    #ifndef NDEBUG
    //    if (uv.y > 1 && (SPIRE_VOXEL_FACE_POS_Z == voxelFace || SPIRE_VOXEL_FACE_NEG_Z == voxelFace)) {
    //        out_Color = vec4(1,1,1,1);
    //        return;
    //    }
    //    #endif

    uint packed = chunkVoxelData[voxelDataAllocationIndex].datas[(voxelDataChunkIndex + voxelDataIndex) / NUM_TYPES_PER_INT];

    uint voxelType = SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, voxelDataIndex);

    // Output debug colour if invalid type
    #ifndef NDEBUG
    if (voxelType == 0) {
        out_Color = vec4(1, 0, 0, 1);
        return;
    }
    #endif

    // Get the image to use
    uint imageIndex = voxelTypesBuffer.voxelTypes[voxelType].FirstTextureIndex + GetImageIndex(voxelTypesBuffer.voxelTypes[voxelType].VoxelFaceLayout, voxelFace);

    out_Color = texture(texSampler[imageIndex], uv);
}