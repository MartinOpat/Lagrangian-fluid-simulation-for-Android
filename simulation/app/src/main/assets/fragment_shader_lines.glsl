#version 320 es
precision mediump float;

flat in vec4 col;
out vec4 fragColor;

void main() {
    fragColor = col;
}

