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

uniform vec3 nearPoint;
uniform vec3 farPoint;
uniform vec3 nfNormal;

uniform float a;
uniform float b;
uniform float c;

void main(void)
{
    // get fragment world space position
    vec3 fragpos = screenCorner + gl_FragCoord.x * rightPerPixel + gl_FragCoord.y * upPerPixel;
    
    vec3 direction = fragpos - viewer0Pos;
    direction = normalize(direction);

    vec2 weight;
    weight.x = acos(dot(direction, viewer0Dir));

    direction = fragpos - viewer1Pos;
    direction = normalize(direction);

    weight.y = acos(dot(direction, viewer1Dir));
    
    //weight = weight * weight * -0.1823784 + weight * -0.095493 + 1.0;
    weight = weight * weight * a + weight * b + c;
    weight = max(weight, 0.0);

    if(weight.x + weight.y <= 0)
    {
	discard;
    }

    float p = weight.y / (weight.x + weight.y);
    p = clamp(p, minRatio, maxRatio);
    float globalp = p;
    if(minRatio == maxRatio)
    {
	p = 1.0;
    }
    else
    {
	p = (p - minRatio) / (maxRatio - minRatio);
    }

    // calculate near/far point for fragment
    vec3 linepoint1,linepoint2;
    vec3 linePoint = mix(viewer0Pos,viewer1Pos,globalp);
    vec3 lineNorm = fragpos - linePoint;
    lineNorm = normalize(lineNorm);

    float d = dot(lineNorm,nfNormal);

    // line parallel to screen?
    if(d == 0.0)
    {
	discard;
    }

    float mult = dot((nearPoint - linePoint),nfNormal) / d;
    linepoint1 = (mult * lineNorm) + linePoint;

    mult = dot((farPoint - linePoint),nfNormal) / d;
    linepoint2 = (mult * lineNorm) + linePoint;

    // find intersection between line and triangle
    mat3 planepoint = mat3(linepoint1.xyz - linepoint2.xyz, pos1 - pos0, pos2 - pos0);
    planepoint = inverse(planepoint);

    vec3 result = linepoint1.xyz - pos0;
    result = planepoint * result;

    // line segment does not intersect triangle plane
    if(result.x < 0.0 || result.x > 1.0)
    {
        discard;
    }

    // intersection point not within triangle
    if(result.y < 0 || result.z < 0 || result.y + result.z > 1.0)
    {
    	discard;
    }

    float trueDepth = result.x * (far - near) + near;
    trueDepth = -trueDepth;
    trueDepth = ((far + near) / (far - near) * trueDepth + 2.0 * far * near / (far - near)) / trueDepth;
    gl_FragDepth = (trueDepth + 1.0) / 2.0;

    // calculate final barycentric coord
    float bcoord0 = 1.0 - result.y - result.z;

    float d0 = mix(diffuseV0P0, diffuseV1P0, p);
    float d1 = mix(diffuseV0P1, diffuseV1P1, p);
    float d2 = mix(diffuseV0P2, diffuseV1P2, p);

    // set weighted frag color
    gl_FragColor.rgb = bcoord0 * d0 * color0 + result.y * d1 * color1 + result.z * d2 * color2;
    gl_FragColor.a = 1.0;
}
