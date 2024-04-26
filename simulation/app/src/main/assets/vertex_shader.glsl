attribute vec4 vPosition;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle
uniform vec3 uPosition;

void main() {
    if (uIsPoint) {
        gl_PointSize = 10.0;
    }
    gl_Position = vPosition;
}
