#version 410 core

in vec2 tex_pos;
out vec4 color;

uniform sampler2D texture0;

void main()
{
    color = texture(texture0, tex_pos);
}