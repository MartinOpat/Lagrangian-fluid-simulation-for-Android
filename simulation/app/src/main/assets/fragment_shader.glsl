#version 300 es
precision mediump float;
uniform bool uIsPoint; // Uniform to tell if we're rendering a point or triangle
uniform sampler2D uTexture; // Texture for the point sprite

in vec2 texCoords;
out vec4 fragColor;

void main() {
    if (uIsPoint) {
        fragColor = texture(uTexture, texCoords);
    } else {
        fragColor = vec4(0.4, 0.5, 0.8, 1.0);  // Blue for now
    }
}

