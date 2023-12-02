R"(
#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 tex_coords;

out vec2 texPos;
out vec4 vertColor;

uniform vec2 spriteSize;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    vec4 sprite_pos = vec4(position.x, position.y * spriteSize.y / spriteSize.x, 0.0, 1.0);
    vec4 final_pos = proj * view * model * sprite_pos;

    gl_Position = vec4(final_pos.xy, 0.0, 1.0);
    texPos = tex_coords;
    vertColor = color;
}

)"
