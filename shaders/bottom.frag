#version 410 core

in vec2 texPos;
in vec4 vertColor;

out vec4 color;

uniform vec2 velocity;

uniform sampler2D texture0;

void main()
{
    color = vertColor * texture(texture0, texPos);
    
    if (color.a <= 0.01) discard;
    if (length(velocity) == 0.0) discard;
    else color = vec4((normalize(velocity).xy + vec2(1.0, 1.0)) / 2.0, 0, 1);
}