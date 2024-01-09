#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 i_POSITION;
layout(location = 1) in vec3 i_COLOUR;
layout(location = 2) in vec3 i_NORMAL;
layout(location = 3) in vec2 i_UV;

layout(location = 0) out vec3 o_VERTEX_COLOUR;
layout(location = 1) out vec2 o_UV;

layout(set = 0, binding = 0) uniform GlobalUbo_t {
    mat4 projection;
    mat4 view;
} u_GLOBAL;

layout(set = 0, binding = 1) uniform ModelUbo_t {
    mat4 transform;
} u_MODEL;

void main() {
    gl_Position = u_GLOBAL.projection * u_GLOBAL.view * u_MODEL.transform * vec4(i_POSITION, 1.0);

    o_VERTEX_COLOUR = i_COLOUR;
    o_UV = i_UV;
}
