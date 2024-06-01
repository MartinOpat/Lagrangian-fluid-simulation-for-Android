//
// Created by martin on 01-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TASK_SCHEDULER_H
#define LAGRANGIAN_FLUID_SIMULATION_TASK_SCHEDULER_H

#include <functional>
#include <queue>
#include <mutex>
#include <thread>


class TaskScheduler {
public:
    TaskScheduler();
    ~TaskScheduler();

    void workerThreadFunction();
    void scheduleTask(const std::function<void()>& task);
    void terminateWorkerThread();

private:
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks;
    bool finished = false;
    std::thread loadThread;
};

#endif //LAGRANGIAN_FLUID_SIMULATION_TASK_SCHEDULER_H
