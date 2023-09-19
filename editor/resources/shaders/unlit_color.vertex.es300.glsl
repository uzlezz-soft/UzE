#version 300 es

layout(std140) uniform _22_24
{
    layout(row_major) mat4 _m0;
    layout(row_major) mat4 _m1;
} _24;

layout(location = 0) in vec3 _64;
layout(location = 1) in vec2 _68;
out vec3 _79;
out vec2 _83;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _97 = spvWorkaroundRowMajor(_24._m0) * vec4(_64, 1.0);
    gl_Position = vec3((spvWorkaroundRowMajor(_24._m1) * vec4(_97.xyz, 1.0)).xyz);
    _79 = _97.xyz;
    _83 = _68;
}

