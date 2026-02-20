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
layout (location = 6) flat in uint faceWidth;
layout (location = 7) flat in uint faceHeight;

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
    // Find voxel index
    // Get the coordinates of the voxel in the current face
    // e.g. for a 1x2 face, valid coordinates are 0,0 and 0,1
    uvec2 voxelCoordsInFace = uvec2(clamp(uv.x, 0, float(faceWidth) - 1), clamp(uv.y, 0, float(faceHeight) - 1));

    // These components of the voxel coords need to be flipped because UV 0,0 isn't always the bottom left (because otherwise certain faces would have flipped textures)
    // it is *probably* better to flip them here rather than passing even more data to the fragment shader
    if (voxelFace == SPIRE_VOXEL_FACE_POS_X || voxelFace == SPIRE_VOXEL_FACE_NEG_Z) {
        voxelCoordsInFace.x = (faceWidth - 1u) - voxelCoordsInFace.x;
    }

    if (voxelFace == SPIRE_VOXEL_FACE_POS_Y) {
        voxelCoordsInFace.y = (faceHeight - 1u) - voxelCoordsInFace.y;
    }

    // Now we can convert from 2D coordinates into 1D axis
    uint voxelIndexInFace = uint(voxelCoordsInFace.y * uint(faceWidth) + voxelCoordsInFace.x);

    uint voxelDataIndex = voxelIndexInFace + voxelTypesFaceStartIndex;// Add on where the data of the current face starts

    // Get voxel type
    uint packed = chunkVoxelData[voxelDataAllocationIndex].datas[(voxelDataChunkIndex + voxelDataIndex) / NUM_TYPES_PER_INT];
    uint voxelType = SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, voxelDataIndex);

    // Debug output
    #ifndef NDEBUG
    if (voxelCoordsInFace.x > 63 || voxelCoordsInFace.y > 63) {
        out_Color = vec4(0.5, 0, 0, 1.0);
        return;
    }

    if (voxelType == 0) {
        out_Color = vec4(1, 0, 0, 1);
        return;
    }
    #endif

    // Get the image to use
    uint imageIndex = voxelTypesBuffer.voxelTypes[voxelType].FirstTextureIndex + GetImageIndex(voxelTypesBuffer.voxelTypes[voxelType].VoxelFaceLayout, voxelFace);

    out_Color = texture(texSampler[imageIndex], uv);
}