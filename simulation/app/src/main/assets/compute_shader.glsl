#version 320 es
layout(local_size_x = 128) in;

// Uniforms for dimensions and time step
uniform int width;
uniform int height;
uniform int depth;
uniform float global_time_in_step;
uniform float TIME_STEP_IN_SECONDS;
uniform float dt;

layout(std430, binding = 0) buffer Particles {
    float particles[]; // x, y, z positions of particles
};

layout(std430, binding = 1) buffer VectorField0 {
    float vectorData0[]; // vector field data
};

layout(std430, binding = 2) buffer VectorField1 {
    float vectorData1[]; // vector field data
};

vec3 getVelocity(vec3 position) {
    // Transform position to grid indices
    int gridX = int((position.x / 100.0f + 1.0f) / 2.0f * float(width));
    int gridY = int((position.y / 100.0f + 1.0f) / 2.0f * float(height));
    int gridZ = 0;

    gridX = clamp(gridX, 0, width - 1);
    gridY = clamp(gridY, 0, height - 1);
    gridZ = clamp(gridZ, 0, depth - 1);

    int idx = gridZ * width * height + gridY * width + gridX;

    // Compute velocity based on vector field data
    vec3 v0 = vec3(vectorData0[idx * 6 + 3] - vectorData0[idx * 6],
    vectorData0[idx * 6 + 4] - vectorData0[idx * 6 + 1],
    vectorData0[idx * 6 + 5] - vectorData0[idx * 6 + 2]);

    vec3 v1 = vec3(vectorData1[idx * 6 + 3] - vectorData1[idx * 6],
    vectorData1[idx * 6 + 4] - vectorData1[idx * 6 + 1],
    vectorData1[idx * 6 + 5] - vectorData1[idx * 6 + 2]);

    // Linear interpolation based on time step
    return v0 + global_time_in_step / TIME_STEP_IN_SECONDS * (v1 - v0);
}

vec3 bindPosition(vec3 position) {
    return vec3(clamp(position.x, -100.0f, 100.0f),
    clamp(position.y, -100.0f, 100.0f),
    clamp(position.z, -50.0f, 50.0f));
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

