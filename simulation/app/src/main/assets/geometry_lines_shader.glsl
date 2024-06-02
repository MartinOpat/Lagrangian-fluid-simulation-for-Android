#version 320 es

#define PI 3.14159265

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

vec3 angleToRGB(float angle) {
    float normalized = mod(angle / (2.0 * PI), 1.0);
    float r = 0.5 + 0.3 * sin(normalized * 2.0 * PI + 0.0);            // Red component
    float g = 0.5 + 0.3 * sin(normalized * 2.0 * PI + 2.0 * PI / 3.0); // Green component
    float b = 0.5 + 0.3 * sin(normalized * 2.0 * PI + 4.0 * PI / 3.0); // Blue component
    float saturation = 0.75;  // Reduce saturation for less intense colors
    vec3 color = vec3(r, g, b);
    vec3 gray = vec3(0.5);  // Gray level for desaturation effect
    color = mix(gray, color, saturation);
    return color;
}

void main() {
    // Get position
    vec3 startPosition = pos[0];
    vec3 endPosition = pos[1];

    // Transform position
    vec4 worldStartPos = modelTransform * vec4(startPosition, 1.0);
    vec4 worldEndPos = modelTransform * vec4(endPosition, 1.0);

    // Get angle
    vec3 v = normalize(endPosition - startPosition);
    float angle = atan(v.y, v.x);

    // Get color
    col = vec4(angleToRGB(angle), 1.0);

    // Emit vertices
    gl_Position = projectionTransform * viewTransform * worldStartPos;
    EmitVertex();

    gl_Position = projectionTransform * viewTransform * worldEndPos;
    EmitVertex();

    EndPrimitive();
}
