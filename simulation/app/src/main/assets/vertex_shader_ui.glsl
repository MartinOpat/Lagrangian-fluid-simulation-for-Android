#version 320 es

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;

uniform mat4 modelTransform;
uniform mat4 viewTransform;

out vec4 col;


void main() {
    gl_Position = viewTransform * modelTransform * vPosition;
    col = vColor;
}
