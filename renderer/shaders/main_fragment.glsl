#version 450

#pragma shader_stage(fragment)

layout(location = 0) in vec3 in_vertex_colour;

layout(location = 0) out vec4 out_colour;

void main() {
    out_colour = vec4(in_vertex_colour, 1.0);
}
