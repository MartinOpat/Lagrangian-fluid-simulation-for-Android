#version 320 es

layout(points) in;
layout(points, max_vertices = 1) out;

in vec3 pos[];
flat out vec4 col;

uniform float uPointSize;
uniform mat4 projectionTransform;
uniform mat4 viewTransform;
uniform mat4 modelTransform;

void main() {
    gl_PointSize = uPointSize;
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(pos[0], 1.0);
    EmitVertex();
    EndPrimitive();
}
