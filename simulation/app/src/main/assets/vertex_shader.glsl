#version 320 es
precision highp float;

out vec2 texCoords;


uniform mat4 projectionTransform;
uniform mat4 viewTransform;
uniform mat4 modelTransform;

void main() {
    const vec3 positions[6] = vec3[](
    vec3(-1.0, -1.0, 0.0), vec3(1.0, -1.0, 0.0), vec3(-1.0, 1.0, 0.0),
    vec3(-1.0, 1.0, 0.0), vec3(1.0, -1.0, 0.0), vec3(1.0, 1.0, 0.0)
    );
    gl_Position = projectionTransform * viewTransform * modelTransform *  vec4(positions[gl_VertexID], 1.0);
    texCoords = positions[gl_VertexID].xy * 0.5 + 0.5;  // Normalized texture coordinates
}
