#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;

out vs_out {
    vec2 uv;
    vec4 col;
} o;

uniform mat4 osg_ModelViewProjectionMatrix;

void main() {
    o.uv = uv;
    o.col = color;
    gl_Position = osg_ModelViewProjectionMatrix * vec4(vertex, 1.0);
}