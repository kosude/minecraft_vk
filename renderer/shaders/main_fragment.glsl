#version 450

#pragma shader_stage(fragment)

layout(location = 0) in vec3    i_VERTEX_COLOUR;
layout(location = 1) in float   i_COLOUR_MULTIPLIER;

layout(location = 0) out vec4   o_COLOUR;

void main() {
    float time = asin(i_COLOUR_MULTIPLIER);

    o_COLOUR = vec4(i_VERTEX_COLOUR * vec3(sin(time), sin(time + 2), sin(time + 1)), 1.0);
}
