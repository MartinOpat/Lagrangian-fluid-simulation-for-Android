#version 300 es
in vec4 vPosition;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle

void main() {
    if (uIsPoint) {
        gl_PointSize = 10.0;
    }
    gl_Position = vPosition;
}
