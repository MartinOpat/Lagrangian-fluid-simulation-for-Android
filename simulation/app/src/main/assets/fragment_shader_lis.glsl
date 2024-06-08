#version 320 es
precision mediump float;
precision mediump sampler3D;  // Using 3D sampler for the vector field texture

in vec2 texCoords;  // Texture coordinates passed from the vertex shader
uniform sampler2D noiseTex;
uniform sampler3D vectorFieldTex;  // 3D texture storing the vector field
out vec4 fragColor;

void main() {
    float licResult = 0.0;
    int steps = 20;
    float stepSize = 1.0 / 512.0;  // Normalization based on texture size

    // Integrate across multiple depth slices
    int depthLayers = 5;  // Number of depth layers to sample
    float depthStep = 1.0 / float(depthLayers);
    for (int d = 0; d < depthLayers; d++) {
        float currentDepth = depthStep * float(d);
        vec3 vector = texture(vectorFieldTex, vec3(texCoords, currentDepth)).rgb;  // Sample vector field at varying depths

        // LIC calculation along the 2D slice of the vector field
        vec2 currentPos = texCoords;
        for (int i = -steps; i <= steps; ++i) {
            vec2 offset = float(i) * stepSize * normalize(vector.xy);  // Use normalized vector for direction
            vec2 samplePos = currentPos + offset;
            samplePos = vec2(fract(samplePos.x), fract(samplePos.y)); // Ensure wrapping around edges
            licResult += texture(noiseTex, samplePos).r;
        }
    }

    licResult /= (float(depthLayers) * (2.0 * float(steps) + 1.0));  // Normalize by number of samples across all depths
    fragColor = vec4(licResult, licResult, licResult, 1.0); // Output the LIC result as grayscale
}
