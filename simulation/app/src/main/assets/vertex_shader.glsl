#version 320 es

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vPosition2;

out vec3 pos;
out vec3 pos2;


void main() {
    pos = vPosition;
    pos2 = vPosition2;
}
