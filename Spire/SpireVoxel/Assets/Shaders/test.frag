#version 460

#include "PushConstants.glsl"
#include "ShaderInfo.h"

layout(location = 0) in vec2 uv;
layout(location = 1) flat in int shouldDiscard;

layout(location = 0) out vec4 out_Color;

layout(set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_SHADER_BINDINGS_MODEL_IMAGES_BINDING) uniform sampler2D texSampler[SPIRE_SHADER_TEXTURE_COUNT];

void main() {
  if (shouldDiscard != 0) discard;
  else {
    out_Color = texture(texSampler[0], uv);
  }
}