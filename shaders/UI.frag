#version 460

#pragma import_defines ( USE_TEXTURE )

out vec4 FragColor;

in vs_out {
	vec2 uv;
    vec4 col;
} i;

flat in vec2 scale;

uniform sampler2D Texture;
uniform float percentRounding;
uniform float absoluteRounding;

float roundRect(vec2 coords, vec2 extents)
{
    vec2 roundAmount = percentRounding * extents + vec2(absoluteRounding, absoluteRounding);
    vec2 spher = vec2(0.0, 0.0);
    if(coords.x < roundAmount.x)
    {
        spher.x = (roundAmount.x - coords.x) / roundAmount.x;
    }
    else if(coords.x > (extents.x - roundAmount.x))
    {
        spher.x = 1.0 + (coords.x - extents.x) / roundAmount.x;
    }

    if(coords.y < roundAmount.y)
    {
        spher.y = (roundAmount.y - coords.y) / roundAmount.y;
    }
    else if(coords.y > (extents.y - roundAmount.y))
    {
        spher.y = 1.0 + (coords.y - extents.y) / roundAmount.y;
    }

    return dot(spher, spher) > 1.0 ? 0.0 : 1.0;
}

void main() {
#ifdef USE_TEXTURE
    vec4 texCol = texture2D(Texture, i.uv);

	FragColor = texCol * i.col;
#else
    FragColor = i.col;
#endif
    FragColor.a *= roundRect(i.uv * scale, scale);
}