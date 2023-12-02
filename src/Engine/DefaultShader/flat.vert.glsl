R"(
#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 tex_coords;

out vec2 texPos;
out vec4 vertColor;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(position, 1.0);
    texPos = tex_coords;
    vertColor = color;
}

)"
