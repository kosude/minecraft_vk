#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec3 i_VERTEX_COLOUR;
layout(location = 1) in vec2 i_UV;

layout(location = 0) out vec4 o_COLOUR;

layout(set = 0, binding = 2) uniform sampler2D u_SAMPLER;

void main() {
    o_COLOUR = texture(u_SAMPLER, i_UV);
}
