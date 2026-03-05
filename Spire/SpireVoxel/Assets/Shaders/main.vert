#version 460
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) flat out int glVertexIndex;
layout (location = 1) flat out int glInstanceIndex;

void main() {
    glVertexIndex = gl_VertexIndex;
    glInstanceIndex = gl_InstanceIndex;
}