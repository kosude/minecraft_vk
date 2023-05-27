#version 450

#pragma shader_stage(vertex)

layout(location = 0) in vec2    i_POSITION;
layout(location = 1) in vec3    i_COLOUR;

layout(location = 0) out vec3   o_VERTEX_COLOUR;
layout(location = 1) out float  o_COLOUR_MULTIPLIER;

layout(binding = 0) uniform     UniformBuffer_t {
    float colour_multiplier;

    mat4 transform_model;
    mat4 transform_view;
    mat4 transform_proj;
} u_UNIFORM;

void main() {
    gl_Position = u_UNIFORM.transform_proj * u_UNIFORM.transform_view * u_UNIFORM.transform_model * vec4(i_POSITION, 0.0, 1.0);

    o_VERTEX_COLOUR = i_COLOUR;
    o_COLOUR_MULTIPLIER = u_UNIFORM.colour_multiplier;
}
