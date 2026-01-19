#version 460
#extension GL_EXT_nonuniform_qualifier : require

#include "PushConstants.glsl"
#include "ShaderInfo.h"

layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 voxelData;
layout (location = 2) flat in uint voxelDataChunkIndex;
layout (location = 3) flat in uint voxelFace;

layout(location = 0) out vec4 out_Color;

layout(set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING) uniform sampler2D texSampler[];

layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_VOXEL_DATA_BINDING) readonly buffer ChunkVoxelData {
    GPUChunkVoxelData datas[];
} chunkVoxelData;

layout (set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING) readonly buffer VoxelTypesBuffer {
    GPUVoxelType voxelTypes[];
} voxelTypesBuffer;

#define UVEC4_LENGTH 4

uint roundToUint(float x) {
    return uint(floor(x));
}

void main() {
    uint voxelDataIndex = SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(roundToUint(voxelData.x), roundToUint(voxelData.y), roundToUint(voxelData.z));
    uvec4 possibleVoxelTypes = chunkVoxelData.datas[voxelDataChunkIndex].Data[voxelDataIndex / UVEC4_LENGTH];
    uint voxelType = possibleVoxelTypes[voxelDataIndex % UVEC4_LENGTH];

    uint imageIndex = voxelTypesBuffer.voxelTypes[voxelType].FirstTextureIndex + GetImageIndex(voxelTypesBuffer.voxelTypes[voxelType].VoxelFaceLayout, voxelFace);

    out_Color = texture(texSampler[imageIndex], uv);
}