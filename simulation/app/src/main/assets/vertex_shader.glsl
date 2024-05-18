#version 320 es

layout(location = 0) in vec3 vPosition;

out vec3 pos;


void main() {
    pos = vPosition;
}
