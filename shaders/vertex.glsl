#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 tex_coords;

out vec2 texPos;
out vec4 vertColor;
out vec2 world_pos;

uniform vec2 spriteSize;

uniform mat4 camera_proj;
uniform mat4 camera_view;
uniform mat4 model;
uniform mat4 MVP;

void main()
{
    vec4 temp = camera_proj * camera_view * model * vec4(position, 1.0);
    world_pos = temp.xy / temp.w;

    gl_Position = MVP * vec4(position, 1.0);
    texPos = tex_coords;
    vertColor = color;
}