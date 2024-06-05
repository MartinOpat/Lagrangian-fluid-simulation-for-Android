#version 320 es
layout(local_size_x = 256) in;

// Uniforms for dimensions and time step
uniform int width;
uniform int height;
uniform int depth;
uniform float global_time_in_step;
uniform float TIME_STEP_IN_SECONDS;
uniform float dt;
uniform float max_width;
uniform float max_height;
uniform float max_depth;

layout(std430, binding = 0) buffer Particles {
    float particles[]; // x, y, z positions of particles
};

layout(std430, binding = 1) buffer VectorField0 {
    float vectorData0[]; // vector field data
};

layout(std430, binding = 2) buffer VectorField1 {
    float vectorData1[]; // vector field data
};

// Helper functions to calculate velocity vector at a given index
vec3 computeVelocity0(int x, int y, int z) {
    int idx = z * width * height + y * width + x;
    return vec3(
        vectorData0[idx * 6 + 3] - vectorData0[idx * 6],
        vectorData0[idx * 6 + 4] - vectorData0[idx * 6 + 1],
        vectorData0[idx * 6 + 5] - vectorData0[idx * 6 + 2]
    );
}
vec3 computeVelocity1(int x, int y, int z) {
    int idx = z * width * height + y * width + x;
    return vec3(
        vectorData1[idx * 6 + 3] - vectorData1[idx * 6],
        vectorData1[idx * 6 + 4] - vectorData1[idx * 6 + 1],
        vectorData1[idx * 6 + 5] - vectorData1[idx * 6 + 2]
    );
}

// Helper functions to interpolate velocity vectors
vec3 interpolateV0(int baseGridX, int baseGridY, int baseGridZ, float w_x, float w_y, float w_z) {
    vec3 v0 = mix(
        mix(
            mix(
                computeVelocity0(baseGridX, baseGridY, baseGridZ),
                computeVelocity0(baseGridX + 1, baseGridY, baseGridZ),
                w_x
            ),
            mix(
                computeVelocity0(baseGridX, baseGridY + 1, baseGridZ),
                computeVelocity0(baseGridX + 1, baseGridY + 1, baseGridZ),
                w_x
            ),
            w_y
        ),
        mix(
            mix(
                computeVelocity0(baseGridX, baseGridY, baseGridZ + 1),
                computeVelocity0(baseGridX + 1, baseGridY, baseGridZ + 1),
                w_x
            ),
            mix(
                computeVelocity0(baseGridX, baseGridY + 1, baseGridZ + 1),
                computeVelocity0(baseGridX + 1, baseGridY + 1, baseGridZ + 1),
                w_x
            ),
            w_y
        ),
        w_z
    );
    return v0;
}
vec3 interpolateV1(int baseGridX, int baseGridY, int baseGridZ, float w_x, float w_y, float w_z) {
    vec3 v1 = mix(
        mix(
            mix(
                computeVelocity1(baseGridX, baseGridY, baseGridZ),
                computeVelocity1(baseGridX + 1, baseGridY, baseGridZ),
                w_x
            ),
            mix(
                computeVelocity1(baseGridX, baseGridY + 1, baseGridZ),
                computeVelocity1(baseGridX + 1, baseGridY + 1, baseGridZ),
                w_x
            ),
            w_y
        ),
        mix(
            mix(
                computeVelocity1(baseGridX, baseGridY, baseGridZ + 1),
                computeVelocity1(baseGridX + 1, baseGridY, baseGridZ + 1),
                w_x
            ),
            mix(
                computeVelocity1(baseGridX, baseGridY + 1, baseGridZ + 1),
                computeVelocity1(baseGridX + 1, baseGridY + 1, baseGridZ + 1),
                w_x
            ),
            w_y
        ),
        w_z
    );

    return v1;
}

vec3 getVelocity(vec3 position) {
    // Transform position to grid indices as floating point
    float fGridX = ((position.x / max_width + 1.0f) / 2.0f * float(width));
    float fGridY = ((position.y / max_height + 1.0f) / 2.0f * float(height));
    float fGridZ = ((position.z / max_depth + 1.0f) / 2.0f * float(depth));

    // Calculate base indices by casting to int
    int baseGridX = int(fGridX);
    int baseGridY = int(fGridY);
    int baseGridZ = int(fGridZ);

    // Ensure base indices are within bounds
    baseGridX = clamp(baseGridX, 0, width - 2);
    baseGridY = clamp(baseGridY, 0, height - 2);
    baseGridZ = clamp(baseGridZ, 0, depth - 2);

    // Compute interpolation weights
    float w_x = fGridX - float(baseGridX);
    float w_y = fGridY - float(baseGridY);
    float w_z = fGridZ - float(baseGridZ);

    // Interpolate velocity vectors in space
    vec3 v0 = interpolateV0(baseGridX, baseGridY, baseGridZ, w_x, w_y, w_z);
    vec3 v1 = interpolateV1(baseGridX, baseGridY, baseGridZ, w_x, w_y, w_z);

    // Linear interpolation based on time step
    return mix(v0, v1, global_time_in_step / TIME_STEP_IN_SECONDS);
}


vec3 bindPosition(vec3 position) {
    return vec3(clamp(position.x, -max_width, max_width),
    clamp(position.y, -max_height, max_height),
    clamp(position.z, -max_depth, max_depth));
}

vec3 advectionStep(vec3 position, float dt) {
    vec3 v1 = getVelocity(position);
    vec3 pos1 = position + 0.5f * v1 * dt;
    vec3 v2 = getVelocity(pos1);
    vec3 pos2 = position + 0.5f * v2 * dt;
    vec3 v3 = getVelocity(pos2);
    vec3 pos3 = position + v3 * dt;
    vec3 v4 = getVelocity(pos3);

    return position + (v1 + 2.0f * v2 + 2.0f * v3 + v4) * dt / 6.0f;
}

void main() {
    int id = int(gl_GlobalInvocationID.x) * 3;
    if (id >= particles.length()) return;

    vec3 position = vec3(particles[id], particles[id + 1], particles[id + 2]);
    position = advectionStep(position, dt);
    position = bindPosition(position);

    // Write back updated position
    particles[id] = position.x;
    particles[id + 1] = position.y;
    particles[id + 2] = position.z;
}

