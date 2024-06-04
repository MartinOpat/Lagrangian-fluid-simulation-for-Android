#version 320 es
precision mediump float;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or not

flat in vec4 col;
out vec4 fragColor;

void main() {
    if (uIsPoint) {
        vec2 center = vec2(0.5, 0.5);
        float radius = 0.1;
        float dist = length(gl_PointCoord - center);

        if (dist > radius) {
            discard;
        } else {
            fragColor = vec4(1.0, 0, 0, 1.0);  // Red
        }
    } else {
        fragColor = col;
    }
}

