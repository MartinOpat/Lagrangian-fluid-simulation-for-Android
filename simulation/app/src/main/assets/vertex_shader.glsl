#version 300 es
in vec4 vPosition;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle

out vec2 texCoords;

void main() {
    if (uIsPoint) {
        gl_PointSize = 10.0;
    }
    gl_Position = vPosition;
    texCoords = vPosition.xy;
}
