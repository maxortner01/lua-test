R"(

#version 410 core

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec2 tex_pos;

out vec4 vertexColor;
out vec2 texPos;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(position.x, position.y * -1.0, position.z, 1.0);
    texPos = tex_pos;
    vertexColor = color;
}

)"