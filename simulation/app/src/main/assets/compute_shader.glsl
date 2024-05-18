//#version 320 es
//layout(local_size_x = 128) in; // Define work-group size
//
//layout(std430, binding = 0) buffer ParticleBuffer {
//    float particles[]; // Flat array of particle positions (x, y, z, x, y, z, ...)
//};
//
//void main() {
//    int id = int(gl_GlobalInvocationID.x) * 3; // Calculate index for x component of each particle
//    if (id + 2 >= particles.length()) return;
//
//    // Example physics update for the particle position
//    vec3 velocity = vec3(0.01f, 0.01f, 0.01f); // Simple constant velocity for demonstration
//    particles[id]   += velocity.x * 2.0f; // Update x
//    particles[id+1] += velocity.y * 2.0f; // Update y
//    particles[id+2] += velocity.z * 2.0f; // Update z
//}

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
    return vec3(clamp(position.x, -float(width), float(width)),
    clamp(position.y, -float(height), float(height)),
    clamp(position.z, -float(depth), float(depth)));
}

void main() {
    int id = int(gl_GlobalInvocationID.x) * 3;
    if (id >= particles.length()) return;

    vec3 position = vec3(particles[id], particles[id + 1], particles[id + 2]);
    vec3 velocity = getVelocity(position);


    // Update particle position as an example (RK4 steps should be added here)
    position += velocity * dt; // Example update
    position = bindPosition(position);

    // Write back updated position
    particles[id] = position.x;
    particles[id + 1] = position.y;
    particles[id + 2] = position.z;
}

