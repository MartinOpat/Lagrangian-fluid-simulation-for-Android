//
// Created by martin on 09-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_CONSTS_H
#define LAGRANGIAN_FLUID_SIMULATION_CONSTS_H

// Simulated field dimensions
#define FIELD_WIDTH 100.0f
#define FIELD_HEIGHT 100.0f
#define FIELD_DEPTH 50.0f

// Grid dimensions
extern int grid_width;
extern int grid_height;
extern int grid_depth;

// Near/far rendering planes
#define NEAR_FAR 500.0f

// Number of particles (only used when not specifying positions from file)
#define NUM_PARTICLES 250000

// Number of simulation time between time steps (two files interpolation) == 1 day
#define TIME_STEP 50

// [0, TIME_STEP)
extern float global_time_in_step;

// Application mode
enum class Mode {
    sequential,
    parallel,
    computeShaders
};
extern Mode mode;

//// Application state - user (i.e. programmer) defined
//struct appState;
//extern appState *globalAppState;

#endif //LAGRANGIAN_FLUID_SIMULATION_CONSTS_H
