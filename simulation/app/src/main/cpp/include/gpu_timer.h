//
// Created by martin on 12-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_GPU_TIMER_H
#define LAGRANGIAN_FLUID_SIMULATION_GPU_TIMER_H

class GpuTimer {
public:
    GpuTimer();
    ~GpuTimer();
    void start();
    void stop();
    double elapsed_milliseconds();
    void logElapsedTime();

};

#endif //LAGRANGIAN_FLUID_SIMULATION_GPU_TIMER_H
