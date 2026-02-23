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
layout (location = 7) flat in uint aoDataChunkPackedIndex;
layout (location = 8) flat in uint aoDataAllocationIndex;

layout(location = 0) out vec4 out_Color;

// All the textures used by voxels
layout(set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING) uniform sampler2D texSampler[];

// The types of all the voxels in the current world
layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_VOXEL_DATA_BINDING) readonly buffer ChunkVoxelData {
    uint datas[];
} chunkVoxelData[];

// Ambient occlusion information
layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_AO_DATA_BINDING) readonly buffer ChunkAOData {
    uint datas[];
} chunkAOData[];

// Information about all registered voxel types
layout (set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING) readonly buffer VoxelTypesBuffer {
    GPUVoxelType voxelTypes[];
} voxelTypesBuffer;

#define NUM_TYPES_PER_INT 2u// 2 voxel types (u16) in a u32

uint roundToUint(float x) {
    return uint(floor(x));
}

#define AO_VALUES_PER_FACE 4

void main() {
    // Get voxel type
    uvec2 voxelCoordsInFace = uvec2(uv);
    uint voxelIndexInFace = uint(voxelCoordsInFace.y * faceWidth + voxelCoordsInFace.x);

    uint voxelDataIndex = voxelIndexInFace + voxelTypesFaceStartIndex;

    uint packedVoxelType = chunkVoxelData[voxelDataAllocationIndex].datas[(voxelDataChunkIndex + voxelDataIndex) / NUM_TYPES_PER_INT];

    uint voxelType = SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packedVoxelType, voxelDataIndex);

    // Ambient occlusion
    uint aoIndex = voxelDataIndex * AO_VALUES_PER_FACE;
    uint packedAO = chunkAOData[aoDataAllocationIndex].datas[aoDataChunkPackedIndex + aoIndex / SPIRE_AO_VALUES_PER_U32];

    uint ao0 = UnpackAO(packedAO, aoIndex % SPIRE_AO_VALUES_PER_U32);
    uint ao1 = UnpackAO(packedAO, (aoIndex + 1) % SPIRE_AO_VALUES_PER_U32);
    uint ao2 = UnpackAO(packedAO, (aoIndex + 2) % SPIRE_AO_VALUES_PER_U32);
    uint ao3 = UnpackAO(packedAO, (aoIndex + 3) % SPIRE_AO_VALUES_PER_U32);

    #ifndef NDEBUG
    // Output debug colour if invalid type
    if (voxelType == 0) {
        out_Color = vec4(1, 0, 0, 1);
        return;
    }

    // Output debug colour if AO is wrong
    if (ao0 > 3 || ao1 > 3 || ao2 > 3 || ao3 > 3) {
        out_Color = vec4(1, 0, 1, 1);
        return;
    }
    #endif

    // Get the image to use
    uint imageIndex = voxelTypesBuffer.voxelTypes[voxelType].FirstTextureIndex + GetImageIndex(voxelTypesBuffer.voxelTypes[voxelType].VoxelFaceLayout, voxelFace);

    out_Color = texture(texSampler[imageIndex], uv);

    // Apply AO
    vec2 voxelUV = fract(uv);

    const float aoStrength = 0.75;

    vec2 f = voxelUV;

    float a0 = float(3 - ao0) / 4.0;
    float a1 = float(3 - ao1) / 4.0;
    float a2 = float(3 - ao2) / 4.0;
    float a3 = float(3 - ao3) / 4.0;

    float w0 = (1.0 - f.x) * (1.0 - f.y);
    float w1 = f.x * (1.0 - f.y);
    float w2 = f.x * f.y;
    float w3 = (1.0 - f.x) * f.y;

    float ao = a0*w0 + a1*w1 + a2*w2 + a3*w3;

    float shade = 1.0 - ao * aoStrength;

    out_Color.xyz *= shade;


}