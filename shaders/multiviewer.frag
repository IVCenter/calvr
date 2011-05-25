#version 150 compatibility
#extension GL_ARB_gpu_shader5 : enable

// world space bottom left corner
uniform vec3 screenCorner;
// world space delta per vertical pixel
uniform vec3 upPerPixel;
// world space delta per horizontal pixel
uniform vec3 rightPerPixel;

// world space location of viewers
uniform vec3 viewer0Pos;
uniform vec3 viewer1Pos;
uniform vec3 viewer0Dir;
uniform vec3 viewer1Dir;

uniform float vwidth;
uniform float vheight;
uniform float near;
uniform float far;

flat in vec3 color0;
flat in vec3 color1;
flat in vec3 color2;

flat in vec3 pos0;
flat in vec3 pos1;
flat in vec3 pos2;

flat in float diffuseV0P0;
flat in float diffuseV0P1;
flat in float diffuseV0P2;
flat in float diffuseV1P0;
flat in float diffuseV1P1;
flat in float diffuseV1P2;

uniform float minRatio;
uniform float maxRatio;

void main(void)
{
    gl_FragColor.xyz = color0;
    //gl_FragColor = vec4(1.0,1.0,1.0,1.0);
    gl_FragColor.w = 1.0;
    //gl_FragDepth = 0.9;
    return;

    // get fragment world space position
    vec3 fragpos = screenCorner + gl_FragCoord.x * rightPerPixel + gl_FragCoord.y * upPerPixel;
    
    vec3 direction = fragpos - viewer0Pos;
    direction = normalize(direction);

    vec2 weight;
    weight.x = acos(dot(direction, viewer0Dir));

    direction = fragpos - viewer1Pos;
    direction = normalize(direction);

    weight.y = acos(dot(direction, viewer1Dir));
    
    weight = weight * weight * -0.1823784 + weight * -0.095493 + 1.0;
    weight = max(weight, 0.0);

    if(weight.x + weight.y <= 0)
    {
	discard;
        if(gl_FragDepth > 0.85)
        {
           gl_FragDepth = 0.9;
	   return;
        }
        else
        {
           discard;
        }
    }

    float p = weight.y / (weight.x + weight.y);
    p = clamp(p, minRatio, maxRatio);

    if(minRatio == maxRatio)
    {
	p = 1.0;
    }
    else
    {
	p = (p - minRatio) / (maxRatio - minRatio);
    }

    //gl_FragColor = vec4(1.0,0.0,0.0,1.0);
    //gl_FragColor.rgb = color0;
    //gl_FragColor.r = weight.x;
    //gl_FragColor.g = weight.y;
    //gl_FragColor.r = (weight.x - 0.9) * 10.0;
    //gl_FragColor.r = p;
    //return;

    // calculate near/far point for fragment
    vec4 linepoint1 = gl_FragCoord;
    linepoint1.x = ((2.0 * linepoint1.x / vwidth) - 1.0) * near;
    linepoint1.y = ((2.0 * linepoint1.y / vheight) - 1.0) * near;
    linepoint1.w = near;

    vec4 linepoint2 = gl_FragCoord;
    linepoint2.x = ((2.0 * linepoint2.x / vwidth) - 1.0) * far;
    linepoint2.y = ((2.0 * linepoint2.y / vheight) - 1.0) * far;
    linepoint2.w = far;

    // projection inverse
    mat4 invmat = (1-p) * gl_TextureMatrixInverse[5] + p * gl_TextureMatrixInverse[7];

    linepoint1 = invmat * linepoint1;
    linepoint2 = invmat * linepoint2;

    linepoint1.w = 1.0;
    linepoint2.w = 1.0;

    // view inverse
    invmat = (1-p) * gl_TextureMatrixInverse[4] + p * gl_TextureMatrixInverse[6];

    linepoint1 = invmat * linepoint1;
    linepoint2 = invmat * linepoint2;

    // find intersection between line and triangle
    mat3 planepoint = mat3(linepoint1.xyz - linepoint2.xyz, pos1 - pos0, pos2 - pos0);
    planepoint = inverse(planepoint);

    vec3 result = linepoint1.xyz - pos0;
    result = planepoint * result;

    // line segment does not intersect triangle plane
    if(result.x < 0.0 || result.x > 1.0)
    {
	//gl_FragColor = vec4(1.0,0.0,0.0,1.0);
	//return;
        discard;
        if(gl_FragDepth > 0.85)
        {
           gl_FragDepth = 0.9;
	   return;
        }
        else
        {
           discard;
        }
    }

    // intersection point not within triangle
    if(result.y < 0 || result.z < 0 || result.y + result.z > 1.0)
    {
	//gl_FragColor = vec4(1.0,0.0,0.0,1.0);
	//return;
    	discard;
        if(gl_FragDepth > 0.85)
        {
           gl_FragDepth = 0.9;
	   return;
        }
        else
        {
           discard;
        }
    }

    // find intersection point
    linepoint1 = linepoint1 + (linepoint2 - linepoint1) * result.x;

    // set depth for fragment
    linepoint1 = ((1-p) * gl_TextureMatrix[5] + p * gl_TextureMatrix[7]) * ((1-p) * gl_TextureMatrix[4] + p * gl_TextureMatrix[6]) * linepoint1;
    
    /*float depth = ((linepoint1.z / linepoint1.w) + 1.0) / 2.0;
    if(depth > gl_FragDepth && gl_FragDepth != 0.5)
    {
        discard;
    }
    gl_FragDepth = depth;*/
    gl_FragDepth = ((linepoint1.z / linepoint1.w) + 1.0) / 2.0;

    // calculate final barycentric coord
    float bcoord0 = 1.0 - result.y - result.z;

    float d0 = mix(diffuseV0P0, diffuseV1P0, p);
    float d1 = mix(diffuseV0P1, diffuseV1P1, p);
    float d2 = mix(diffuseV0P2, diffuseV1P2, p);
    //float diffuse = bcoord0 * d0 + result.y * d1 + result.z * d2;

    // set weighted frag color
    //gl_FragColor.rgb = bcoord0 * color0 + result.y * color1 + result.z * color2;
    gl_FragColor.rgb = bcoord0 * d0 * color0 + result.y * d1 * color1 + result.z * d2 * color2;
    //gl_FragColor = vec4(1.0,1.0,1.0,1.0);
    gl_FragColor.a = 1.0;

    //gl_FragColor = vec4(1.0,0.0,0.0,1.0);
    //gl_FragColor.rgb = color0;
}
