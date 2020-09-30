#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;

out vs_out {
    vec2 uv;
    vec4 col;
} o;

flat out vec2 scale;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ViewMatrixInverse;
uniform mat4 osg_ModelViewMatrix;

void main() {
    o.uv = uv;
    o.col = color;

    mat4 world = osg_ViewMatrixInverse * osg_ModelViewMatrix;
    scale.x = length(world * vec4(1.0, 0.0, 0.0, 0.0));
    scale.y = length(world * vec4(0.0, 0.0, 1.0, 0.0));

    gl_Position = osg_ModelViewProjectionMatrix * vec4(vertex, 1.0);
}