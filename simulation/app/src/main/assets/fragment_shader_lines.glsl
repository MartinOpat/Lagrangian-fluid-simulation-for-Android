#version 320 es
precision mediump float;

in vec4 col;
in float angle;

uniform sampler2D colorMap;

out vec4 fragColor;

void main() {
//    fragColor = texture(colorMap, vec2(angle, 0.0));
    fragColor = col;
}

