#version 460
#extension GL_EXT_nonuniform_qualifier : require

#include "PushConstants.glsl"
#include "ShaderInfo.h"

layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING) readonly buffer Vertices {
	VertexData data[];
} in_Vertices[];

layout (set = SPIRE_SHADER_BINDINGS_PER_FRAME_SET, binding = SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING) readonly uniform CameraBuffer { mat4 ViewProjectionMatrix; } cameraBuffer;

struct ModelData
{
	mat4 ModelMatrix;
};

//layout (set = SPIRE_SHADER_BINDINGS_PER_FRAME_SET, binding = SPIRE_SHADER_BINDINGS_MODEL_DATA_SSBO_BINDING) readonly buffer ModelDataBuffer { ModelData modelData[]; } modelDataBuffer;

layout (location = 0) out vec2 texCoord;

void main()
{
	VertexData vtx = in_Vertices[gl_InstanceIndex].data[gl_VertexIndex];

	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

	gl_Position = cameraBuffer.ViewProjectionMatrix * vec4(pos, 1.0);

	texCoord = vec2(vtx.u, vtx.v);
	texCoord.x *= 0.5;

	if (vtx.VoxelType == 2) texCoord.x += 0.5;
}