#version 300 es
in vec4 vPosition;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle
uniform float uPointSize;

void main() {
    if (uIsPoint) {
        gl_PointSize = uPointSize;
    }
    gl_Position = vPosition;
}
