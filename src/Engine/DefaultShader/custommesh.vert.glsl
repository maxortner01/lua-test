R"(
#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

out vec4 vertColor;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(position, 1.0);
    vertColor = color;
}

)"
