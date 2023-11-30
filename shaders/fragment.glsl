uniform vec2 texture_size;
uniform sampler2D texture;

void main() 
{ 
  gl_FragColor = gl_Color;
}

/*uniform sampler2D texture;

uniform vec3 camera_pos;
uniform vec2 size;
uniform vec2 target_size;

void main()
{
    vec2 pos = vec2(
        gl_FragCoord.x - (320.0 * 3.0 / 2.0) + camera_pos.x,
        gl_FragCoord.y + (320.0 * 3.0 / 2.0) - camera_pos.y
    );

    vec4 pixel = texture2D(texture, vec2(pos.x / size.x, pos.y / size.y));
    if (pixel.a > 0.0) discard;
    gl_FragColor = vec4(0, 1, 0, 1);
}*/