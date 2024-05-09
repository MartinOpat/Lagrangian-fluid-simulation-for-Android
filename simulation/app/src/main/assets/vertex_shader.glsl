#version 300 es
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNextPos;
flat out vec4 col;
out float hue;

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
    }
//    gl_Position = vPosition;
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPosition, 1.0f);
    vec4 v = vec4(vNextPos - vPosition, 0.0f);
    float angle = atan(v.y, v.x);
    hue = (angle + 3.14159) / (2.0 * 3.14159);
    col = vec4(hsv2rgb(vec3(hue, 1.0, 1.0)), 1.0);

//    col = normalize(vPosition);
//    col = normalize(vNextPos);
//    col = 0.5*normalize(vNextPos - vPosition) + 0.5;
//    col = vPosition*0.5 + 0.5;
//    gl_Position = projectionTransform * viewTransform * modelTransform * vPosition;
}
