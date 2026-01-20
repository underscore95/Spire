#version 460
#extension GL_EXT_nonuniform_qualifier : require

#include "PushConstants.glsl"
#include "ShaderInfo.h"

// See vertex shader for input documentation
layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 voxelData;
layout (location = 2) flat in uint voxelDataChunkIndex;
layout (location = 3) flat in uint voxelFace;

layout(location = 0) out vec4 out_Color;

// All the textures used by voxels
layout(set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING) uniform sampler2D texSampler[];

// The types of all the voxels in the current world
layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_VOXEL_DATA_BINDING) readonly buffer ChunkVoxelData {
    GPUChunkVoxelData datas[];
} chunkVoxelData;

// Information about all registered voxel types
layout (set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING) readonly buffer VoxelTypesBuffer {
    GPUVoxelType voxelTypes[];
} voxelTypesBuffer;

#define UVEC4_LENGTH 4u
#define NUM_TYPES_PER_INT 2u// 2 voxel types (u16) in a u32

uint roundToUint(float x) {
    return uint(floor(x));
}

void main() {
    // Get the voxel type
    uint voxelDataIndex = SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(roundToUint(voxelData.x), roundToUint(voxelData.y), roundToUint(voxelData.z));

    uint uvec4Index = voxelDataIndex / (UVEC4_LENGTH * NUM_TYPES_PER_INT);
    uint uintIndex  = (voxelDataIndex / NUM_TYPES_PER_INT) % UVEC4_LENGTH;
    uint halfIndex  = voxelDataIndex % NUM_TYPES_PER_INT;

    uint packed = chunkVoxelData.datas[voxelDataChunkIndex].Data[uvec4Index][uintIndex];

    uint voxelType = SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, voxelDataIndex);

#ifndef NDEBUG
    if (voxelType == 0) {
        out_Color = vec4(1, voxelType == 0 ? 1 : 0, 0, 1);
        return;
    }
#endif

    // Get the image to use
    uint imageIndex = voxelTypesBuffer.voxelTypes[voxelType].FirstTextureIndex + GetImageIndex(voxelTypesBuffer.voxelTypes[voxelType].VoxelFaceLayout, voxelFace);

    out_Color = texture(texSampler[imageIndex], uv);
}