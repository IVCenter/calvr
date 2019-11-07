#version 460

#pragma import_defines ( USE_TEXTURE )

out vec4 FragColor;

in vs_out {
	vec2 uv;
    vec4 col;
} i;

uniform sampler2D Texture;

void main() {
#ifdef USE_TEXTURE
    vec4 texCol = texture2D(Texture, i.uv);

	FragColor = texCol * i.col;
#else
    FragColor = i.col;
#endif
}