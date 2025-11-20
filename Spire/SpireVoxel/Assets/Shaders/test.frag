#version 460
#extension GL_EXT_nonuniform_qualifier : require

#include "PushConstants.glsl"
#include "ShaderInfo.h"

layout(location = 0) in vec2 uv;
layout(location = 1) flat in int shouldDiscard;
layout(location = 2) flat in uint imageIndex;

layout(location = 0) out vec4 out_Color;

layout(set = SPIRE_SHADER_BINDINGS_CONSTANT_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING) uniform sampler2D texSampler[];

void main() {
    if (shouldDiscard != 0) discard;
    else {
        out_Color = texture(texSampler[imageIndex], uv);
    }
}