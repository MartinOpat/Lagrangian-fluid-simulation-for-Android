#version 300 es
precision mediump float;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle

flat in vec4 col;
out vec4 fragColor;

void main() {
    if (uIsPoint) {
        vec2 center = vec2(0.5, 0.5);
        float radius = 0.5;
        float dist = length(gl_PointCoord - center);

        if (dist > radius) {
            discard;
        } else {
//            fragColor = vec4(1.0 + 10.0 * depth, 0, 0, 1.0);  // Red for now
            fragColor = vec4(1.0, 0, 0, 1.0);  // Red for now
        }
    } else {
        // fragColor = vec4(0.4, 0.5, 0.8, 1.0);  // Blue for now
        fragColor = col;
    }
}

