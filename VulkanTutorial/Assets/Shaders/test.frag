#version 460

#include "PushConstants.glsl"

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 out_Color;

layout(set = 0, binding = 2) uniform sampler2D texSampler[2];

void main() {
  out_Color = texture(texSampler[pushConstants.TextureIndex], uv);
}