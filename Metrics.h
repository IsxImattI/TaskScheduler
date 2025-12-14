#ifndef METRICS_H
#define METRICS_H

#include <windows.h>
#include <iostream>

class Metrics {
private:
    // atomic counters using InterlockedXxx functions
    volatile LONG totalTasksEnqueued;
    volatile LONG totalTasksCompleted;
    volatile LONG activeTasks;
    
    // timing
    LARGE_INTEGER frequency;
    LARGE_INTEGER startTime;
    
    CRITICAL_SECTION cs;
    
public:
    Metrics() : totalTasksEnqueued(0), totalTasksCompleted(0), activeTasks(0) {
        InitializeCriticalSection(&cs);
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&startTime);
    }
    
    ~Metrics() {
        DeleteCriticalSection(&cs);
    }
    
    // increment counters (thread-safe using Interlocked)
    void taskEnqueued() {
        InterlockedIncrement(&totalTasksEnqueued);
    }
    
    void taskStarted() {
        InterlockedIncrement(&activeTasks);
    }
    
    void taskCompleted() {
        InterlockedDecrement(&activeTasks);
        InterlockedIncrement(&totalTasksCompleted);
    }
    
    // getters
    LONG getTotalEnqueued() const {
        return totalTasksEnqueued;
    }
    
    LONG getTotalCompleted() const {
        return totalTasksCompleted;
    }
    
    LONG getActiveTasks() const {
        return activeTasks;
    }
    
    LONG getPendingTasks() const {
        return totalTasksEnqueued - totalTasksCompleted;
    }
    
    // calculate throughput (tasks per second)
    double getThroughput() const {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        
        double elapsedSeconds = (double)(currentTime.QuadPart - startTime.QuadPart) / frequency.QuadPart;
        
        if (elapsedSeconds == 0) return 0;
        return (double)totalTasksCompleted / elapsedSeconds;
    }
    
    // get elapsed time in seconds
    double getElapsedTime() const {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        return (double)(currentTime.QuadPart - startTime.QuadPart) / frequency.QuadPart;
    }
    
    // print stats
    void printStats() const {
        EnterCriticalSection((LPCRITICAL_SECTION)&cs);
        
        std::cout << "\n=== METRICS ===" << std::endl;
        std::cout << "Total Enqueued:  " << totalTasksEnqueued << std::endl;
        std::cout << "Total Completed: " << totalTasksCompleted << std::endl;
        std::cout << "Active Tasks:    " << activeTasks << std::endl;
        std::cout << "Pending Tasks:   " << getPendingTasks() << std::endl;
        std::cout << "Throughput:      " << getThroughput() << " tasks/sec" << std::endl;
        std::cout << "Elapsed Time:    " << getElapsedTime() << " sec" << std::endl;
        std::cout << "===============\n" << std::endl;
        
        LeaveCriticalSection((LPCRITICAL_SECTION)&cs);
    }
};

#endif