#version 300 es

#define PI 3.141593

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNextPos;
flat out vec4 col;

uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle
uniform float uPointSize;

uniform mat4 projectionTransform;
uniform mat4 viewTransform;
uniform mat4 modelTransform;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    if (uIsPoint) {
        gl_PointSize = uPointSize;
        gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPosition, 1.0f);
    } else {
        gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPosition, 1.0f);

        vec4 v = vec4(vNextPos - vPosition, 0.0f);
        float angle = atan(v.y, v.x);
        float hue = (angle + PI) / (2.0 * PI);
        col = vec4(hsv2rgb(vec3(hue, 1.0, 1.0)), 1.0);
    }
}
