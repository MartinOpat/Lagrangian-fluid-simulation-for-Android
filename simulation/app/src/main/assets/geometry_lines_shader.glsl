#version 320 es

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

in vec3 pos[];
flat out vec4 col;

uniform mat4 projectionTransform;
uniform mat4 viewTransform;
uniform mat4 modelTransform;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    vec3 startPosition = pos[0];
    vec3 endPosition = pos[1];

    vec4 worldStartPos = modelTransform * vec4(startPosition, 1.0);
    vec4 worldEndPos = modelTransform * vec4(endPosition, 1.0);

    vec3 v = normalize(endPosition - startPosition);
    float angle = atan(v.y, v.x);
    float hue = (angle + 3.141593) / (2.0 * 3.141593);
    col = vec4(hsv2rgb(vec3(hue, 1.0, 1.0)), 1.0);

    // Output the transformed vertices and pass color to fragment shader
    gl_Position = projectionTransform * viewTransform * worldStartPos;
    EmitVertex();

    gl_Position = projectionTransform * viewTransform * worldEndPos;
    EmitVertex();

    EndPrimitive();
}
