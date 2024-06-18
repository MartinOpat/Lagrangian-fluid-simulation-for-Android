//
// Created by martin on 09-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_CONSTS_H
#define LAGRANGIAN_FLUID_SIMULATION_CONSTS_H

// Simulated field dimensions
#define FIELD_WIDTH 100.0f
#define FIELD_HEIGHT 100.0f
#define FIELD_DEPTH 50.0f

// Near/far rendering planes
#define NEAR_FAR 100.0f

// Number of simulation time between time steps (two files interpolation) == 1 day
#define TIME_STEP 10

// [0, TIME_STEP)
extern float global_time_in_step;

// Application mode
enum class Mode {
    sequential,
    parallel,
    computeShaders
};
extern Mode mode;

// Number of particles (only used when not specifying positions from file)
extern int num_particles;

#endif //LAGRANGIAN_FLUID_SIMULATION_CONSTS_H
