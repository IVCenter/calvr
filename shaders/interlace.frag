#version 150 compatibility

uniform sampler2D texture;

void main(void)
{
    //gl_FragColor = vec4(1.0,0.0,0.0,1.0);
    ivec2 size = textureSize(texture, 0);
    int x = int(gl_FragCoord.x);
    int y = size.y - int(gl_FragCoord.y) - 1;
    if(bool(y % 2))
    {
	y = (y + size.y) / 2;
    }
    else
    {
	y = y / 2;
    }
    gl_FragColor = texelFetch(texture,ivec2(x,y),0);
}
