#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 vert_color;

out vec4 color;

void main()
{   
    color = vert_color;
}