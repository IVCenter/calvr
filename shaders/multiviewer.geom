#version 150 compatibility
#extension GL_EXT_geometry_shader4 : enable
#extension GL_ARB_gpu_shader5 : enable

uniform float viewer0Dist;
uniform float viewer1Dist;

uniform float near;

flat out vec3 color0;
flat out vec3 color1;
flat out vec3 color2;

flat out vec3 pos0;
flat out vec3 pos1;
flat out vec3 pos2;

in vec3 normals[3];

flat out float diffuseV0P0;
flat out float diffuseV0P1;
flat out float diffuseV0P2;
flat out float diffuseV1P0;
flat out float diffuseV1P1;
flat out float diffuseV1P2;

void main(void)
{
    pos0 = gl_PositionIn[0].xyz;
    pos1 = gl_PositionIn[1].xyz;
    pos2 = gl_PositionIn[2].xyz;

    color0 = gl_FrontColorIn[0].rgb;
    color1 = gl_FrontColorIn[1].rgb;
    color2 = gl_FrontColorIn[2].rgb;

    //color0 = vec3(0.0,0.0,1.0);
    //color1 = vec3(0.0,1.0,0.0);
    //color2 = vec3(1.0,0.0,0.0);

    vec4 t0p0 = gl_TextureMatrix[4] * gl_PositionIn[0];
    vec4 t0p1 = gl_TextureMatrix[4] * gl_PositionIn[1];
    vec4 t0p2 = gl_TextureMatrix[4] * gl_PositionIn[2];

    vec4 t1p0 = gl_TextureMatrix[6] * gl_PositionIn[0];
    vec4 t1p1 = gl_TextureMatrix[6] * gl_PositionIn[1];
    vec4 t1p2 = gl_TextureMatrix[6] * gl_PositionIn[2];

    vec4 lightPos = gl_TextureMatrix[4] * gl_LightSource[0].position;
    vec3 ptol;
    vec4 normal;

    normal = vec4(normals[0],0.0);
    normal = gl_ModelViewMatrixInverseTranspose * normal;
    normal.w = 0.0;
    normal = gl_TextureMatrixInverseTranspose[4] * normal;
    normal.xyz = normalize(normal.xyz);
    ptol = normalize(lightPos.xyz - t0p0.xyz);
    diffuseV0P0 = dot(ptol, normal.xyz);

    normal = vec4(normals[1],0.0);
    normal = gl_ModelViewMatrixInverseTranspose * normal;
    normal.w = 0.0;
    normal = gl_TextureMatrixInverseTranspose[4] * normal;
    normal.xyz = normalize(normal.xyz);
    ptol = normalize(lightPos.xyz - t0p1.xyz);
    diffuseV0P1 = dot(ptol, normal.xyz);

    normal = vec4(normals[2],0.0);
    normal = gl_ModelViewMatrixInverseTranspose * normal;
    normal.w = 0.0;
    normal = gl_TextureMatrixInverseTranspose[4] * normal;
    normal.xyz = normalize(normal.xyz);
    ptol = normalize(lightPos.xyz - t0p2.xyz);
    diffuseV0P2 = dot(ptol, normal.xyz);

    lightPos = gl_TextureMatrix[6] * gl_LightSource[0].position;
    
    normal = vec4(normals[0],0.0);
    normal = gl_ModelViewMatrixInverseTranspose * normal;
    normal.w = 0.0;
    normal = gl_TextureMatrixInverseTranspose[6] * normal;
    normal.xyz = normalize(normal.xyz);
    ptol = normalize(lightPos.xyz - t1p0.xyz);
    diffuseV1P0 = dot(ptol, normal.xyz);

    normal = vec4(normals[1],0.0);
    normal = gl_ModelViewMatrixInverseTranspose * normal;
    normal.w = 0.0;
    normal = gl_TextureMatrixInverseTranspose[6] * normal;
    normal.xyz = normalize(normal.xyz);
    ptol = normalize(lightPos.xyz - t1p1.xyz);
    diffuseV1P1 = dot(ptol, normal.xyz);

    normal = vec4(normals[2],0.0);
    normal = gl_ModelViewMatrixInverseTranspose * normal;
    normal.w = 0.0;
    normal = gl_TextureMatrixInverseTranspose[6] * normal;
    normal.xyz = normalize(normal.xyz);
    ptol = normalize(lightPos.xyz - t1p2.xyz);
    diffuseV1P2 = dot(ptol, normal.xyz);

    if(diffuseV0P0 <= 0 && diffuseV0P0 <= 0 && diffuseV0P0 <= 0 && diffuseV0P0 <= 0 && diffuseV0P0 <= 0 && diffuseV0P0 <= 0)
    {
	//return;
    }

    diffuseV0P0 = max(0.0, diffuseV0P0);
    diffuseV0P1 = max(0.0, diffuseV0P1);
    diffuseV0P2 = max(0.0, diffuseV0P2);
    diffuseV1P0 = max(0.0, diffuseV1P0);
    diffuseV1P1 = max(0.0, diffuseV1P1);
    diffuseV1P2 = max(0.0, diffuseV1P2);


    bool behind0 = false;
    bool behind1 = false;

    if(t0p0.z > -near && t0p1.z > -near && t0p2.z > -near)
    {
	behind0 = true;
    }

    if(t1p0.z > -near && t1p1.z > -near && t1p2.z > -near)
    {
	behind1 = true;
    }

    if(behind0 && behind1)
    {
	return;
    }

    float distance0 = t0p0.z;
    float distance1 = t1p0.z;

    if(distance0 > -near || distance1 > -near)
    {
	float ratio = (-near - distance0) / (distance1 - distance0);
	vec4 point = gl_PositionIn[0];
	point = ((1.0 - ratio) * gl_TextureMatrix[4] + ratio * gl_TextureMatrix[6]) * point;
	point = ((1.0 - ratio) * gl_TextureMatrix[5] + ratio * gl_TextureMatrix[7]) * point;
	if(distance0 > -near)
	{
	    t0p0 = point;
	}
	else
	{
	    t0p0 = gl_TextureMatrix[5] * t0p0;
	}
	if(distance1 > -near)
	{
	    t1p0 = point;
	}
	else
	{
	    t1p0 = gl_TextureMatrix[7] * t1p0;
	}
    }
    else
    {
	t0p0 = gl_TextureMatrix[5] * t0p0;
	t1p0 = gl_TextureMatrix[7] * t1p0;
    }


    distance0 = t0p1.z;
    distance1 = t1p1.z;

    if(distance0 > -near || distance1 > -near)
    {
	float ratio = (-near - distance0) / (distance1 - distance0);
	vec4 point = gl_PositionIn[1];
	point = ((1.0 - ratio) * gl_TextureMatrix[4] + ratio * gl_TextureMatrix[6]) * point;
	point = ((1.0 - ratio) * gl_TextureMatrix[5] + ratio * gl_TextureMatrix[7]) * point;
	if(distance0 > -near)
	{
	    t0p1 = point;
	}
	else
	{
	    t0p1 = gl_TextureMatrix[5] * t0p1;
	}
	if(distance1 > -near)
	{
	    t1p1 = point;
	}
	else
	{
	    t1p1 = gl_TextureMatrix[7] * t1p1;
	}
    }
    else
    {
	t0p1 = gl_TextureMatrix[5] * t0p1;
	t1p1 = gl_TextureMatrix[7] * t1p1;
    }

    
    distance0 = t0p2.z;
    distance1 = t1p2.z;

    if(distance0 > -near || distance1 > -near)
    {
	float ratio = (-near - distance0) / (distance1 - distance0);
	vec4 point = gl_PositionIn[2];
	point = ((1.0 - ratio) * gl_TextureMatrix[4] + ratio * gl_TextureMatrix[6]) * point;
	point = ((1.0 - ratio) * gl_TextureMatrix[5] + ratio * gl_TextureMatrix[7]) * point;
	if(distance0 > -near)
	{
	    t0p2 = point;
	}
	else
	{
	    t0p2 = gl_TextureMatrix[5] * t0p2;
	}
	if(distance1 > -near)
	{
	    t1p2 = point;
	}
	else
	{
	    t1p2 = gl_TextureMatrix[7] * t1p2;
	}
    }
    else
    {
	t0p2 = gl_TextureMatrix[5] * t0p2;
	t1p2 = gl_TextureMatrix[7] * t1p2;
    }


    /*t0p0 = gl_TextureMatrix[5] * t0p0;
    t0p1 = gl_TextureMatrix[5] * t0p1;
    t0p2 = gl_TextureMatrix[5] * t0p2;

    t1p0 = gl_TextureMatrix[7] * t1p0;
    t1p1 = gl_TextureMatrix[7] * t1p1;
    t1p2 = gl_TextureMatrix[7] * t1p2;*/

    /*vec4 unt0[3];
    vec4 unt1[3];

    unt0[0] = t0p0;
    unt0[1] = t0p1;
    unt0[2] = t0p2;

    unt1[0] = t1p0;
    unt1[1] = t1p1;
    unt1[2] = t1p2;*/

    t0p0 = t0p0 / t0p0.w;
    t0p1 = t0p1 / t0p1.w;
    t0p2 = t0p2 / t0p2.w;

    t1p0 = t1p0 / t1p0.w;
    t1p1 = t1p1 / t1p1.w;
    t1p2 = t1p2 / t1p2.w;

    vec4 quad[4];

    bool segmentFound = false;
    bool outputTriangle = false;
    bool bypass = false;

    quad[0] = t0p0;
    quad[1] = t0p1;
    //quad[0] = unt0[0];
    //quad[1] = unt0[1];

    vec3 seg0,seg1,seg2;
    seg0 = t1p2.xyz - t0p2.xyz;
    seg0.z = 0;
    seg0 = normalize(seg0);

    seg1 = t0p0.xyz - t0p2.xyz;
    seg1.z = 0;
    seg1 = normalize(seg1);

    seg2 = t0p1.xyz - t0p2.xyz;
    seg2.z = 0;
    seg2 = normalize(seg2);

    if(dot(seg0,seg1) >= 0.0)
    {
	outputTriangle = true;
    }
    else
    {
	outputTriangle = false;
    }

    seg1 = cross(seg0,seg1);
    seg2 = cross(seg0,seg2);

    if(sign(seg1.z) != sign(seg2.z))
    {
	segmentFound = true;
    }

    if(sign(seg1.z) == 0.0 || sign(seg2.z) == 0.0)
    {
	segmentFound = true;
	outputTriangle = true;
	bypass = true;
    }

    if(!segmentFound)
    {
	quad[0] = t0p1;
	quad[1] = t0p2;
	//quad[0] = unt0[1];
	//quad[1] = unt0[2];

	seg0 = t1p0.xyz - t0p0.xyz;
	seg0.z = 0;
	seg0 = normalize(seg0);

	seg1 = t0p1.xyz - t0p0.xyz;
	seg1.z = 0;
	seg1 = normalize(seg1);

	seg2 = t0p2.xyz - t0p0.xyz;
	seg2.z = 0;
	seg2 = normalize(seg2);

	if(dot(seg0,seg1) >= 0.0)
	{
	    outputTriangle = true;
	}
	else
	{
	    outputTriangle = false;
	}

	seg1 = cross(seg0,seg1);
	seg2 = cross(seg0,seg2);

	if(sign(seg1.z) != sign(seg2.z))
	{
	    segmentFound = true;
	}

	if(sign(seg1.z) == 0.0 || sign(seg2.z) == 0.0)
	{
	    segmentFound = true;
	    outputTriangle = true;
	    bypass = true;
	}
    }

    if(!segmentFound)
    {
	quad[0] = t0p2;
	quad[1] = t0p0;
	//quad[0] = unt0[2];
	//quad[1] = unt0[0];

	seg0 = t1p1.xyz - t0p1.xyz;
	seg0.z = 0;
	seg0 = normalize(seg0);

	seg1 = t0p2.xyz - t0p1.xyz;
	seg1.z = 0;
	seg1 = normalize(seg1);

	seg2 = t0p0.xyz - t0p1.xyz;
	seg2.z = 0;
	seg2 = normalize(seg2);

	if(dot(seg0,seg1) >= 0.0)
	{
	    outputTriangle = true;
	}
	else
	{
	    outputTriangle = false;
	}

	seg1 = cross(seg0,seg1);
	seg2 = cross(seg0,seg2);

	if(sign(seg1.z) != sign(seg2.z))
	{
	    segmentFound = true;
	}

	if(sign(seg1.z) == 0.0 || sign(seg2.z) == 0.0)
	{
	    segmentFound = true;
	    outputTriangle = true;
	    bypass = true;
	}
    }

    if(outputTriangle)
    {
	color0 = vec3(1.0,0.0,0.0);
	gl_Position = t0p0;
	EmitVertex();
	gl_Position = t0p1;
	EmitVertex();
	gl_Position = t0p2;
	EmitVertex();
	EndPrimitive();

	/*gl_Position = unt0[0];
	EmitVertex();
	gl_Position = unt0[1];
	EmitVertex();
	gl_Position = unt0[2];
	EmitVertex();
	EndPrimitive();*/
    }

    outputTriangle = false;

    segmentFound = false;

    quad[3] = t1p0;
    quad[2] = t1p1;
    //quad[3] = unt1[0];
    //quad[2] = unt1[1];

    seg0 = t0p2.xyz - t1p2.xyz;
    seg0.z = 0;
    seg0 = normalize(seg0);

    seg1 = t1p0.xyz - t1p2.xyz;
    seg1.z = 0;
    seg1 = normalize(seg1);

    seg2 = t1p1.xyz - t1p2.xyz;
    seg2.z = 0;
    seg2 = normalize(seg2);

    if(dot(seg0,seg1) >= 0.0)
    {
	outputTriangle = true;
    }
    else
    {
        outputTriangle = false;
    }

    seg1 = cross(seg0,seg1);
    seg2 = cross(seg0,seg2);

    if(sign(seg1.z) != sign(seg2.z))
    {
	segmentFound = true;
    }

    if(sign(seg1.z) == 0.0 || sign(seg2.z) == 0.0)
    {
	segmentFound = true;
	outputTriangle = true;
	bypass = true;
    }

    if(!segmentFound)
    {
	quad[3] = t1p1;
	quad[2] = t1p2;
	//quad[3] = unt1[1];
	//quad[2] = unt1[2];

	seg0 = t0p0.xyz - t1p0.xyz;
	seg0.z = 0;
	seg0 = normalize(seg0);

	seg1 = t1p1.xyz - t1p0.xyz;
	seg1.z = 0;
	seg1 = normalize(seg1);

	seg2 = t1p2.xyz - t1p0.xyz;
	seg2.z = 0;
	seg2 = normalize(seg2);

	if(dot(seg0,seg1) >= 0.0)
	{
	    outputTriangle = true;
	}
	else
	{
	    outputTriangle = false;
	}

	seg1 = cross(seg0,seg1);
	seg2 = cross(seg0,seg2);

	if(sign(seg1.z) != sign(seg2.z))
	{
	    segmentFound = true;
	}

	if(sign(seg1.z) == 0.0 || sign(seg2.z) == 0.0)
	{
	    segmentFound = true;
	    outputTriangle = true;
	    bypass = true;
	}
    }

    if(!segmentFound)
    {
	quad[3] = t1p2;
	quad[2] = t1p0;
	//quad[3] = unt1[2];
	//quad[2] = unt1[0];

	seg0 = t0p1.xyz - t1p1.xyz;
	seg0.z = 0;
	seg0 = normalize(seg0);

	seg1 = t1p2.xyz - t1p1.xyz;
	seg1.z = 0;
	seg1 = normalize(seg1);

	seg2 = t1p0.xyz - t1p1.xyz;
	seg2.z = 0;
	seg2 = normalize(seg2);

	if(dot(seg0,seg1) >= 0.0)
	{
	    outputTriangle = true;
	}
	else
	{
	    outputTriangle = false;
	}

	seg1 = cross(seg0,seg1);
	seg2 = cross(seg0,seg2);

	if(sign(seg1.z) != sign(seg2.z))
	{
	    segmentFound = true;
	}

	if(sign(seg1.z) == 0.0 || sign(seg2.z) == 0.0)
	{
	    segmentFound = true;
	    outputTriangle = true;
	    bypass = true;
	}
    }

    if(outputTriangle)
    {
	color0 = vec3(0.0,0.0,1.0);
	gl_Position = t1p0;
	EmitVertex();
	gl_Position = t1p1;
	EmitVertex();
	gl_Position = t1p2;
	EmitVertex();
	EndPrimitive();

	/*gl_Position = unt1[0];
	EmitVertex();
	gl_Position = unt1[1];
	EmitVertex();
	gl_Position = unt1[2];
	EmitVertex();
	EndPrimitive();*/
    }

    if(bypass)
    {
	return;
    }

    color0 = vec3(0.0,1.0,0.0);

    vec3 norm0,norm1;
    seg0.xy = quad[0].xy - quad[1].xy;
    seg1.xy = quad[2].xy - quad[1].xy;
    seg0.z = 0;
    seg1.z = 0;
    norm0 = cross(seg0.xyz,seg1.xyz);
    norm0 = normalize(norm0);

    seg0.xy = quad[2].xy - quad[3].xy;
    seg1.xy = quad[0].xy - quad[3].xy;
    seg0.z = 0;
    seg1.z = 0;
    norm1 = cross(seg0.xyz,seg1.xyz);
    norm1 = normalize(norm1);
    float normDot = dot(norm0,norm1);

    if(normDot < 0)
    {
	vec4 tempvec = quad[0];
	quad[0] = quad[1];
	quad[1] = tempvec;
    }


    /*gl_Position = unt0[0];
    EmitVertex();
    gl_Position = unt1[0];
    EmitVertex();
    gl_Position = unt1[1];
    EmitVertex();
    EndPrimitive();

    gl_Position = unt0[0];
    EmitVertex();
    gl_Position = unt1[1];
    EmitVertex();
    gl_Position = unt0[1];
    EmitVertex();
    EndPrimitive();

    gl_Position = unt0[0];
    EmitVertex();
    gl_Position = unt1[2];
    EmitVertex();
    gl_Position = unt1[0];
    EmitVertex();
    EndPrimitive();

    gl_Position = unt0[0];
    EmitVertex();
    gl_Position = unt0[2];
    EmitVertex();
    gl_Position = unt1[2];
    EmitVertex();
    EndPrimitive();

    gl_Position = unt0[1];
    EmitVertex();
    gl_Position = unt1[1];
    EmitVertex();
    gl_Position = unt1[2];
    EmitVertex();
    EndPrimitive();

    gl_Position = unt0[2];
    EmitVertex();
    gl_Position = unt0[1];
    EmitVertex();
    gl_Position = unt1[2];
    EmitVertex();
    EndPrimitive();

    return;*/


    gl_Position = quad[0];
    EmitVertex();
    gl_Position = quad[1];
    EmitVertex();
    gl_Position = quad[2];
    EmitVertex();
    EndPrimitive();

    gl_Position = quad[2];
    EmitVertex();
    gl_Position = quad[3];
    EmitVertex();
    gl_Position = quad[0];
    EmitVertex();
    EndPrimitive();

    /*float gmaxx, gminx, gmaxy, gminy;

    gmaxx = max(t0p0.x,t0p1.x);
    gmaxx = max(t0p2.x,gmaxx);
    gmaxx = max(t1p0.x,gmaxx);
    gmaxx = max(t1p1.x,gmaxx);
    gmaxx = max(t1p2.x,gmaxx);

    gmaxy = max(t0p0.y,t0p1.y);
    gmaxy = max(t0p2.y,gmaxy);
    gmaxy = max(t1p0.y,gmaxy);
    gmaxy = max(t1p1.y,gmaxy);
    gmaxy = max(t1p2.y,gmaxy);

    gminx = min(t0p0.x,t0p1.x);
    gminx = min(t0p2.x,gminx);
    gminx = min(t1p0.x,gminx);
    gminx = min(t1p1.x,gminx);
    gminx = min(t1p2.x,gminx);

    gminy = min(t0p0.y,t0p1.y);
    gminy = min(t0p2.y,gminy);
    gminy = min(t1p0.y,gminy);
    gminy = min(t1p1.y,gminy);
    gminy = min(t1p2.y,gminy);

    bool flag[3];
    flag[0] = false;
    flag[1] = false;
    flag[2] = false;
    
    int count = 0;

    if(t0p0.x == gminx || t0p0.x == gmaxx || t0p0.y == gmaxy || t0p0.y == gminy)
    {
	flag[0] = true;
	count++;
    }

    if(t0p1.x == gminx || t0p1.x == gmaxx || t0p1.y == gmaxy || t0p1.y == gminy)
    {
	flag[1] = true;
	count++;
    }

    if(t0p2.x == gminx || t0p2.x == gmaxx || t0p2.y == gmaxy || t0p2.y == gminy)
    {
	flag[2] = true;
	count++;
    }

    if(count == 3)
    {
	gl_Position = t0p0;
	EmitVertex();
	gl_Position = t0p1;
	EmitVertex();
	gl_Position = t0p2;
	EmitVertex();
	EndPrimitive();

	return;
    }
    else if(count == 1)
    {
	flag[0] = !flag[0];
	flag[1] = !flag[1];
	flag[2] = !flag[2];
    }

    count = 0;
    bool flag1[3];
    flag1[0] = false;
    flag1[1] = false;
    flag1[2] = false;

    if(t1p0.x == gminx || t1p0.x == gmaxx || t1p0.y == gmaxy || t1p0.y == gminy)
    {
	flag1[0] = true;
	count++;
    }

    if(t1p1.x == gminx || t1p1.x == gmaxx || t1p1.y == gmaxy || t1p1.y == gminy)
    {
	flag1[1] = true;
	count++;
    }

    if(t1p2.x == gminx || t1p2.x == gmaxx || t1p2.y == gmaxy || t1p2.y == gminy)
    {
	flag1[2] = true;
	count++;
    }

    if(count == 3)
    {
	gl_Position = t1p0;
	EmitVertex();
	gl_Position = t1p1;
	EmitVertex();
	gl_Position = t1p2;
	EmitVertex();
	EndPrimitive();

	return;
    }
    else if(count == 1)
    {
	flag1[0] = !flag1[0];
	flag1[1] = !flag1[1];
	flag1[2] = !flag1[2];
    }

    // emit triangles

    gl_Position = t0p0;
    EmitVertex();
    gl_Position = t0p1;
    EmitVertex();
    gl_Position = t0p2;
    EmitVertex();
    EndPrimitive();

    gl_Position = t1p0;
    EmitVertex();
    gl_Position = t1p1;
    EmitVertex();
    gl_Position = t1p2;
    EmitVertex();
    EndPrimitive();

    vec4 quad[4];
    count = 0;

    if(flag[0])
    {
	quad[count] = t0p0;
	count++;
    }

    if(flag[1])
    {
	quad[count] = t0p1;
	count++;
    }

    if(flag[2])
    {
	quad[count] = t0p2;
	count++;
    }

    if(flag1[0])
    {
	quad[count] = t1p0;
	count++;
    }

    if(flag1[1])
    {
	quad[count] = t1p1;
	count++;
    }

    if(flag1[2])
    {
	quad[count] = t1p2;
	count++;
    }

    // untwist points

    vec3 seg0,seg1;
    vec3 norm0,norm1;
    seg0.xy = quad[0].xy - quad[1].xy;
    seg1.xy = quad[2].xy - quad[1].xy;
    seg0.z = 0;
    seg1.z = 0;
    norm0 = cross(seg0.xyz,seg1.xyz);
    norm0 = normalize(norm0);

    seg0.xy = quad[2].xy - quad[3].xy;
    seg1.xy = quad[0].xy - quad[3].xy;
    seg0.z = 0;
    seg1.z = 0;
    norm1 = cross(seg0.xyz,seg1.xyz);
    norm1 = normalize(norm1);
    float normDot = dot(norm0,norm1);

    if(normDot < 0)
    {
	vec4 tempvec = quad[0];
	quad[0] = quad[1];
	quad[1] = tempvec;
    }

    gl_Position = quad[0];
    EmitVertex();
    gl_Position = quad[1];
    EmitVertex();
    gl_Position = quad[2];
    EmitVertex();
    EndPrimitive();

    gl_Position = quad[2];
    EmitVertex();
    gl_Position = quad[3];
    EmitVertex();
    gl_Position = quad[0];
    EmitVertex();
    EndPrimitive();*/

    /*mat4 trans = gl_TextureMatrix[5] * gl_TextureMatrix[4];
    
    vec4 t0p0 = trans * gl_PositionIn[0];
    vec4 t0p1 = trans * gl_PositionIn[1];
    vec4 t0p2 = trans * gl_PositionIn[2];

    //t0p0 = t0p0 / t0p0.w;
    //t0p1 = t0p1 / t0p1.w;
    //t0p2 = t0p2 / t0p2.w;

    trans = gl_TextureMatrix[7] * gl_TextureMatrix[6];

    vec4 t1p0 = trans * gl_PositionIn[0];
    vec4 t1p1 = trans * gl_PositionIn[1];
    vec4 t1p2 = trans * gl_PositionIn[2];

    //t1p0 = t1p0 / t1p0.w;
    //t1p1 = t1p1 / t1p1.w;
    //t1p2 = t1p2 / t1p2.w;*/

    /*float minDepth, maxDepth;

    minDepth = min(t0p0.z, t0p1.z);
    minDepth = min(minDepth, t0p2.z);
    minDepth = min(minDepth, t1p0.z);
    minDepth = min(minDepth, t1p1.z);
    minDepth = min(minDepth, t1p2.z);

    minDepth = max(minDepth, -1.0);

    maxDepth = max(t0p0.z, t0p1.z);
    maxDepth = max(maxDepth, t0p2.z);
    maxDepth = max(maxDepth, t1p0.z);
    maxDepth = max(maxDepth, t1p1.z);
    maxDepth = max(maxDepth, t1p2.z);

    if(maxDepth < -1.0)
    {
	return;
    }

    t0p0.z = minDepth;
    t0p1.z = minDepth;
    t0p2.z = minDepth;
    t1p0.z = minDepth;
    t1p1.z = minDepth;
    t1p2.z = minDepth;*/

    /*gl_Position = vec4(-1.0,-1.0,0.0,1.0);
    EmitVertex();
    gl_Position = vec4(-1.0,1.0,0.0,1.0);
    EmitVertex();
    gl_Position = vec4(1.0,-1.0,0.0,1.0);
    EmitVertex();
    gl_Position = vec4(1.0,1.0,0.0,1.0);
    EmitVertex();

    return;*/

    /*gl_Position = t0p0;
    EmitVertex();
    gl_Position = t0p1;
    EmitVertex();
    gl_Position = t0p2;
    EmitVertex();
    EndPrimitive();

    gl_Position = t0p0;
    EmitVertex();
    gl_Position = t1p0;
    EmitVertex();
    gl_Position = t1p1;
    EmitVertex();
    EndPrimitive();

    gl_Position = t0p0;
    EmitVertex();
    gl_Position = t1p1;
    EmitVertex();
    gl_Position = t0p1;
    EmitVertex();
    EndPrimitive();

    gl_Position = t0p0;
    EmitVertex();
    gl_Position = t1p2;
    EmitVertex();
    gl_Position = t1p0;
    EmitVertex();
    EndPrimitive();

    gl_Position = t0p0;
    EmitVertex();
    gl_Position = t0p2;
    EmitVertex();
    gl_Position = t1p2;
    EmitVertex();
    EndPrimitive();

    gl_Position = t0p1;
    EmitVertex();
    gl_Position = t1p1;
    EmitVertex();
    gl_Position = t1p2;
    EmitVertex();
    EndPrimitive();

    gl_Position = t0p2;
    EmitVertex();
    gl_Position = t0p1;
    EmitVertex();
    gl_Position = t1p2;
    EmitVertex();
    EndPrimitive();*/

    return;
}
