attribute vec4 vPosition;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle

void main() {
    gl_Position = vPosition;

    if (uIsPoint) {
        gl_PointSize = 100.0;
    }
}
