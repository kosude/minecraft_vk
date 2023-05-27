#version 450

#pragma shader_stage(vertex)

layout(location = 0) in vec2    i_POSITION;
layout(location = 1) in vec3    i_COLOUR;

layout(location = 0) out vec3   o_VERTEX_COLOUR;

layout(binding = 0) uniform     UniformBuffer_t {
    mat4 model;
    mat4 view;
    mat4 proj;
} u_UNIFORM;

void main() {
    gl_Position = u_UNIFORM.proj * u_UNIFORM.view * u_UNIFORM.model * vec4(i_POSITION, 0.0, 1.0);

    o_VERTEX_COLOUR = i_COLOUR;
}
