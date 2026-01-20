#version 460
#extension GL_EXT_nonuniform_qualifier : require

#include "PushConstants.glsl"
#include "ShaderInfo.h"

// Vertex buffer
layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING) readonly buffer Vertices {
    VertexData data[];
} in_Vertices[];

// Camera matrices
layout (set = SPIRE_SHADER_BINDINGS_PER_FRAME_SET, binding = SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING) readonly uniform CameraBuffer { CameraInfo cameraInfo; } cameraBuffer;

// Information about all chunks
layout (set = SPIRE_SHADER_BINDINGS_PER_FRAME_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CHUNK_DATA_SSBO_BINDING) readonly buffer ChunkDataBuffer {
    ChunkData chunkDatas[];
} chunkDataBuffer;

layout (location = 0) out vec2 texCoord;// texture coordinate of the vertex
layout (location = 1) out vec3 voxelData;// voxel coordinate in world space
layout (location = 2) flat out uint voxelDataChunkIndex;// Index of the current chunk
layout (location = 3) flat out uint voxelFace;// What face is this vertex part of
layout (location = 4) flat out uint voxelDataAllocationIndex; // Allocation index (what buffer)

void main()
{
    ChunkData chunkData = chunkDataBuffer.chunkDatas[gl_InstanceIndex];
    uint vertexIndex = chunkData.NumVerticesPerBuffer * chunkData.VertexBufferIndex + (gl_VertexIndex % chunkData.NumVerticesPerBuffer);

    VertexData vtx = in_Vertices[chunkData.VertexBufferIndex].data[vertexIndex];

    uint vertexVoxelPos = UnpackVertexDataVertexPosition(vtx.Packed_7X7Y7Z2VertPos3Face);
    uvec3 voxelPos = UnpackVertexDataXYZ(vtx.Packed_7X7Y7Z2VertPos3Face);// position in the chunk
    ivec3 chunkPos = ivec3(chunkData.ChunkX, chunkData.ChunkY, chunkData.ChunkZ);// position of the chunk in chunk-space, so chunk 1,0,0's minimum x voxel is 64
    vec3 worldPos = vec3(voxelPos) + chunkPos * SPIRE_VOXEL_CHUNK_SIZE;// world position of the vertex
    voxelFace = UnpackVertexDataFace(vtx.Packed_7X7Y7Z2VertPos3Face);

    gl_Position = cameraBuffer.cameraInfo.ViewProjectionMatrix * vec4(worldPos, 1.0);
    texCoord = VoxelVertexPositionToUV(vertexVoxelPos) * UnpackVertexFaceWidthHeight(vtx.Packed_6Width6Height);
    voxelData = vec3(voxelPos.x, voxelPos.y, voxelPos.z) - FaceToDirection(voxelFace) * 0.5f /*move to the center of the voxel*/;
    voxelDataChunkIndex = chunkData.VoxelDataChunkIndex;
    voxelDataAllocationIndex = chunkData.VoxelDataAllocationIndex;
}