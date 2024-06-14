#version 320 es
precision mediump float;

in vec4 col;
out vec4 fragColor;

void main() {
    fragColor = col;
}

