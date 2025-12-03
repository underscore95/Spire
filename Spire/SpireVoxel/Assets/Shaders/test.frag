#version 460
#extension GL_EXT_nonuniform_qualifier : require

#include "PushConstants.glsl"
#include "ShaderInfo.h"
#include "VoxelDataHashMap.h"

layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 voxelData;
layout (location = 2) flat in uint voxelFace;
layout (location = 3) flat in uint voxelDataStartingMapIndex;
layout (location = 4) flat in uint voxelDataBucketCount;

layout(location = 0) out vec4 out_Color;

layout(set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING) uniform sampler2D texSampler[];

layout (set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING) readonly buffer VoxelTypesBuffer {
    GPUVoxelType voxelTypes[];
} voxelTypesBuffer;

#define UVEC4_LENGTH 4

uint roundToUint(float x) {
    return uint(floor(x));
}

void main() {
    uint voxelType = VoxelDataHashMapGet(roundToUint(voxelData.x), roundToUint(voxelData.y), roundToUint(voxelData.z), voxelDataStartingMapIndex, voxelDataBucketCount);

    uint imageIndex = voxelTypesBuffer.voxelTypes[voxelType].FirstTextureIndex + GetImageIndex(voxelTypesBuffer.voxelTypes[voxelType].VoxelFaceLayout, voxelFace);

    out_Color = texture(texSampler[imageIndex], uv);
}