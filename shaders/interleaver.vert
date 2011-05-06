
uniform int  eyes;
uniform vec4 size;
uniform vec3 offset;
uniform vec4 coeff[8];

varying vec3 phase[8];

void main()
{
    vec4 pos = gl_Vertex * size;

    mat4 M = mat4(vec4(pos.x + offset.r,
                       pos.x + offset.g,
                       pos.x + offset.b, 0.0),
                  vec4(pos.y),
                  vec4(pos.z),
                  vec4(pos.w));

    phase[0] = (M * coeff[0]).xyz;
    phase[1] = (M * coeff[1]).xyz;
    phase[2] = (M * coeff[2]).xyz;
    phase[3] = (M * coeff[3]).xyz;
    phase[4] = (M * coeff[4]).xyz;
    phase[5] = (M * coeff[5]).xyz;
    phase[6] = (M * coeff[6]).xyz;
    phase[7] = (M * coeff[7]).xyz;

    gl_Position = gl_Vertex;
}
