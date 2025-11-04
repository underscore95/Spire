#version 460

#include "PushConstants.glsl"
#include "ShaderInfo.h"

struct VertexData
{
	float x, y, z;
	float u, v;
};

layout (set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_SHADER_BINDINGS_VERTEX_SSBO_BINDING) readonly buffer Vertices { VertexData data[]; } in_Vertices;

layout (set = SPIRE_SHADER_BINDINGS_PER_FRAME_SET, binding = SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING) readonly uniform CameraBuffer { mat4 ViewProjectionMatrix; } cameraBuffer;

struct ModelData
{
	mat4 ModelMatrix;
};

layout (set = SPIRE_SHADER_BINDINGS_PER_FRAME_SET, binding = SPIRE_SHADER_BINDINGS_MODEL_DATA_SSBO_BINDING) readonly buffer ModelDataBuffer { ModelData modelData[]; } modelDataBuffer;

layout (location = 0) out vec2 texCoord;

void main()
{
	VertexData vtx = in_Vertices.data[gl_VertexIndex];

	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

	gl_Position = cameraBuffer.ViewProjectionMatrix * modelDataBuffer.modelData[gl_InstanceIndex].ModelMatrix * vec4(pos, 1.0);

	texCoord = vec2(vtx.u, vtx.v);
}