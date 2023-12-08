#version 410 core

in vec2 texPos;
in vec4 vertColor;
in vec2 world_pos;

out vec4 color;

uniform sampler2D texture0;

void main()
{
    vec4 tex_color = texture(texture0, (world_pos + vec2(1.0, 1.0)) / 2.0);
    
    vec2 velocity = tex_color.xy * 2.0 - vec2(1.0, 1.0);

    if (tex_color.a >= 0.1) color = vec4(0, 0.25 * abs(velocity.y + velocity.x) / 2.0 + 0.2, 0, 1.0);
    else color = vec4(0, 0.9, 0, 1);
}