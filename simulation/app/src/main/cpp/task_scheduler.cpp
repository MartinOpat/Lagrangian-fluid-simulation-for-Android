//
// Created by martin on 01-06-2024.
//

#include "task_scheduler.h"

TaskScheduler::TaskScheduler() {
    loadThread = std::thread(&TaskScheduler::workerThreadFunction, this);
}

TaskScheduler::~TaskScheduler() {
    terminateWorkerThread();
}

void TaskScheduler::workerThreadFunction() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !tasks.empty() || finished; });
            if (finished && tasks.empty()) break;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

void TaskScheduler::scheduleTask(const std::function<void()>& task) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push(task);
    }
    cv.notify_one();
}

void TaskScheduler::terminateWorkerThread() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_one();
    if (loadThread.joinable()) {
        loadThread.join();
    }
}