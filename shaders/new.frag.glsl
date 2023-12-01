#version 410 core

in vec4 color;
in vec2 texturePos;

out vec4 vertColor;

uniform sampler2D texture0;

void main()
{
    vertColor = color * texture(texture0, texturePos);
}