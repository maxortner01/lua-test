R"(
#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 tex_coords;

out vec2 texPos;
out vec4 vertColor;

uniform vec2 spriteSize;

uniform mat4 MVP;

void main()
{
    vec4 sprite_pos = vec4(position.x * spriteSize.x, position.y * spriteSize.y, position.z, 1.0);

    gl_Position = MVP * sprite_pos;
    texPos = tex_coords;
    vertColor = color;
}

)"
