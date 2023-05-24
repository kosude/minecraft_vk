#version 450

#pragma shader_stage(vertex)

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_colour;

layout(location = 0) out vec3 out_vertex_colour;

void main() {
    gl_Position = vec4(in_position, 0.0, 1.0);

    out_vertex_colour = in_colour;
}
