#version 460
#extension GL_EXT_nonuniform_qualifier : require

#include "PushConstants.glsl"
#include "ShaderInfo.h"

layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING) readonly buffer Vertices {
    VertexData data[];
} in_Vertices;

layout (set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING) readonly buffer VoxelTypesBuffer {
    GPUVoxelType voxelTypes[];
} voxelTypesBuffer;

layout (set = SPIRE_SHADER_BINDINGS_PER_FRAME_SET, binding = SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING) readonly uniform CameraBuffer { CameraInfo cameraInfo; } cameraBuffer;

layout (set = SPIRE_SHADER_BINDINGS_PER_FRAME_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CHUNK_DATA_SSBO_BINDING) readonly buffer ChunkDataBuffer {
    ChunkData chunkDatas[];
} chunkDataBuffer;

layout (location = 0) out vec2 texCoord;
layout (location = 2) flat out uint imageIndex;

void main()
{
    VertexData vtx = in_Vertices.data[gl_VertexIndex];
    ChunkData chunkData = chunkDataBuffer.chunkDatas[gl_InstanceIndex];

    uvec3 voxelPos = UnpackVertexDataXYZ(vtx.Packed_XYZ); // position in the chunk
    ivec3 chunkPos = ivec3(chunkData.ChunkX, chunkData.ChunkY, chunkData.ChunkZ); // position of the chunk in chunk-space, so chunk 1,0,0's minimum x voxel is 64
    vec3 worldPos = vec3(voxelPos) + chunkPos * SPIRE_VOXEL_CHUNK_SIZE; // world position of the voxel

    gl_Position = cameraBuffer.cameraInfo.ViewProjectionMatrix * vec4(worldPos, 1.0);
    texCoord = vec2(vtx.u, vtx.v);
    imageIndex = voxelTypesBuffer.voxelTypes[vtx.VoxelType].FirstTextureIndex + GetImageIndex(voxelTypesBuffer.voxelTypes[vtx.VoxelType].VoxelFaceLayout, vtx.Face);
}