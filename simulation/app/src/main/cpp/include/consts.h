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

// Number of particles (only used when not specifying positions from file)
#define NUM_PARTICLES 25000

// Number of simulation time between time steps (two files interpolation)
#define TIME_STEP_IN_SECONDS 10

// [0, TIME_STEP_IN_SECONDS)
extern float global_time_in_step;

// Application mode
enum class Mode {
    sequential,
    parallel,
    computeShaders
};
extern Mode mode;

#endif //LAGRANGIAN_FLUID_SIMULATION_CONSTS_H
