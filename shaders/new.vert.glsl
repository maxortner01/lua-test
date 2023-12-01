#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 vert_color;
layout (location = 2) in vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 color;
out vec2 texturePos;

void main()
{   
    vec4 pos = vec4(position.xyz, 1.0);
    gl_Position = pos * model * view * projection;
    
    color = vert_color;
    texturePos = texCoords;
}