#version 460

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 out_Color;

layout(binding = 2) uniform sampler2D texSampler;

void main() {
  out_Color = texture(texSampler, uv);
}