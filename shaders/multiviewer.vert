#version 150 compatibility

out vec3 normals;
out vec2 texcoord;

void main(void)
{
    //gl_Position = ftransform();
    //gl_Position = gl_TextureMatrix[5] * gl_TextureMatrix[4] * gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ModelViewMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
    texcoord = gl_MultiTexCoord0.xy;
    normals = gl_Normal;
}
