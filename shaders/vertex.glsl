void main()
{
    gl_Position = gl_Vertex;

    // transform the texture coordinates
    gl_TexCoord[0] = gl_MultiTexCoord0;

    // forward the vertex color
    gl_FrontColor = gl_Color;
}