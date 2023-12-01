#version 410 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texPos;

out vec2 tex_pos;

void main()
{
    gl_Position = vec4(pos.xy, 0.0, 1.0);
    tex_pos = texPos;
}