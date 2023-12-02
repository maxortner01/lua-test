R"(
#version 410 core

in vec2 texPos;
in vec4 vertColor;

out vec4 color;

uniform sampler2D texture0;

void main()
{
    color = vertColor * texture(texture0, texPos);
}

)"
