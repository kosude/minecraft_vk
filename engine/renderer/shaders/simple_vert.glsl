#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 i_POSITION;
layout(location = 1) in vec3 i_COLOUR;
layout(location = 2) in vec3 i_NORMAL;
layout(location = 3) in vec2 i_UV;

layout(location = 0) out vec3 o_VERTEX_COLOUR;

void main() {
    gl_Position = vec4(i_POSITION, 1.0);

    o_VERTEX_COLOUR = i_COLOUR;
}
