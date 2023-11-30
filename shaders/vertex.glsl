uniform mat3 model;
uniform mat3 view;
uniform float camera_z;
uniform float aspectRatio;

void main() 
{ 
    vec3 prepos = gl_Vertex.xyz; 
    prepos.y *= -1.0;
    vec4 pos = vec4((prepos * model).xy, gl_Vertex.z, gl_Vertex.w);
    pos.w *= camera_z;
    pos.x += model[2][0] + view[2][0];
    pos.y += model[2][1] + view[2][1];
    pos.x /= aspectRatio;
    gl_Position = pos; 
    gl_TexCoord[0] = gl_MultiTexCoord0; 
    gl_FrontColor = gl_Color; 
}