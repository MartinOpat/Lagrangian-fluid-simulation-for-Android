#version 300 es
layout(location = 0) in vec4 vPosition;
out float depth;

uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle
uniform float uPointSize;

uniform mat4 projectionTransform;
uniform mat4 viewTransform;
uniform mat4 modelTransform;

void main() {
    if (uIsPoint) {
        gl_PointSize = uPointSize;
    }
    gl_Position = vPosition;
    depth = vPosition.z;
//    gl_Position = projectionTransform * viewTransform * modelTransform * vPosition;
}
