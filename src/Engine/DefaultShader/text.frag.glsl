R"(

#version 410 core

in vec4 vertexColor;
in vec2 texPos;

out vec4 color;

uniform sampler2D texture0;

void main()
{
    vec4 sampled = vec4(0.0, 0.0, 0.0, texture(texture0, texPos).r);
    color = vertexColor * sampled;
    if (sampled.a < 0.1) discard;
}

)"