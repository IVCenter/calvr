#extension GL_ARB_texture_rectangle : enable

uniform float         quality;
uniform int           eyes;

uniform sampler2DRect image[8];
uniform vec3          depth[8];
uniform vec3          edge0[8];
uniform vec3          edge1[8];
uniform vec3          edge2[8];
uniform vec3          edge3[8];
uniform vec3          edge4[8];
uniform vec3          edge5[8];
uniform vec3          edge6[8];

uniform vec2	      frag_d;
uniform vec2          frag_k;

varying vec3          phase[8];

/* Interpolate from (x0, y0) to (x1, y1), giving zero outside [x0, x1] */

vec3 segment(vec3 x0, vec3 x1, vec3 y0, vec3 y1, vec3 t)
{
    return step(x0, t) * step(t, x1) * mix(y0, y1, (t - x0) / (x1 - x0));
}

/* Evaluate the waveform for the given steps, cycle, and depth. */

vec3 wave(vec3 edge0, vec3 edge1, vec3 edge2, vec3 edge3,
          vec3 edge4, vec3 edge5, vec3 edge6, vec3 depth, vec3 phase)
{
    /* Compute each linear waveform segment independantly. */

    vec3 s0 = max(segment(edge0, edge1, edge6, depth, phase), edge0);
    vec3 s1 = min(segment(edge1, edge2, depth, edge0, phase), edge0);
    vec3 s2 = min(segment(edge3, edge4, edge0, depth, phase), edge0);
    vec3 s3 = max(segment(edge4, edge5, depth, edge6, phase), edge0);
    vec3 s4 = max(segment(edge5, edge6, edge6, edge6, phase), edge0);

    /* Accumulate all segments, giving the total waveform. */

    return s0 + s1 + s2 + s3 + s4;
}

void main()
{
    /* Reference all view textures. */

    vec4 C[8];

    vec2 p = (gl_FragCoord.xy  + frag_d) * frag_k;

/*
    C[0] = texture2DRect(image[0], gl_FragCoord.xy * quality);
    C[1] = texture2DRect(image[1], gl_FragCoord.xy * quality);
    C[2] = texture2DRect(image[2], gl_FragCoord.xy * quality);
    C[3] = texture2DRect(image[3], gl_FragCoord.xy * quality);
    C[4] = texture2DRect(image[4], gl_FragCoord.xy * quality);
    C[5] = texture2DRect(image[5], gl_FragCoord.xy * quality);
    C[6] = texture2DRect(image[6], gl_FragCoord.xy * quality);
    C[7] = texture2DRect(image[7], gl_FragCoord.xy * quality);
*/

    C[0] = texture2DRect(image[0], p);
    C[1] = texture2DRect(image[1], p);
    C[2] = texture2DRect(image[2], p);
    C[3] = texture2DRect(image[3], p);
    C[4] = texture2DRect(image[4], p);
    C[5] = texture2DRect(image[5], p);
    C[6] = texture2DRect(image[6], p);
    C[7] = texture2DRect(image[7], p);
    /* Sum the modulation of each image against its waveform. */

    vec3 M[8];

    M[0] = C[0].rgb * wave(edge0[0], edge1[0], edge2[0], edge3[0],
                           edge4[0], edge5[0], edge6[0], depth[0],
                           fract(phase[0]));
    M[1] = C[1].rgb * wave(edge0[1], edge1[1], edge2[1], edge3[1],
                           edge4[1], edge5[1], edge6[1], depth[1],
                           fract(phase[1]));
    M[2] = C[2].rgb * wave(edge0[2], edge1[2], edge2[2], edge3[2],
                           edge4[2], edge5[2], edge6[2], depth[2],
                           fract(phase[2]));
    M[3] = C[3].rgb * wave(edge0[3], edge1[3], edge2[3], edge3[3],
                           edge4[3], edge5[3], edge6[3], depth[3],
                           fract(phase[3]));
    M[4] = C[4].rgb * wave(edge0[4], edge1[4], edge2[4], edge3[4],
                           edge4[4], edge5[4], edge6[4], depth[4],
                           fract(phase[4]));
    M[5] = C[5].rgb * wave(edge0[5], edge1[5], edge2[5], edge3[5],
                           edge4[5], edge5[5], edge6[5], depth[5],
                           fract(phase[5]));
    M[6] = C[6].rgb * wave(edge0[6], edge1[6], edge2[6], edge3[6],
                           edge4[6], edge5[6], edge6[6], depth[6],
                           fract(phase[6]));
    M[7] = C[7].rgb * wave(edge0[7], edge1[7], edge2[7], edge3[7],
                           edge4[7], edge5[7], edge6[7], depth[7],
                           fract(phase[7]));

/*    vec3 revec = (M[0] + M[1] + M[2] + M[3] +
                        M[4] + M[5] + M[6] + M[7]);
    revec = revec * 0.5;
    revec = revec + vec3(0.5, 0.5, 0.5);
    gl_FragColor = vec4(revec, 1.0);*/
    gl_FragColor = vec4(M[0] + M[1] + M[2] + M[3] +
                        M[4] + M[5] + M[6] + M[7], 1.0);
}
