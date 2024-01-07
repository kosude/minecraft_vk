#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec3 i_VERTEX_COLOUR;

layout(location = 0) out vec4 o_COLOUR;

void main() {
    o_COLOUR = vec4(i_VERTEX_COLOUR, 1.0);
}
