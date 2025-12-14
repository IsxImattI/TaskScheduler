#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <windows.h>
#include <iostream>
#include "TaskScheduler.h"
#include "Logger.h"

class Benchmark {
private:
    LARGE_INTEGER frequency;
    
    // simple benchmark task
    static void BenchmarkTask(void* arg) {
        int iterations = *(int*)arg;
        
        // simulate work
        volatile int sum = 0;
        for (int i = 0; i < iterations; i++) {
            sum += i;
        }
    }
    
    // get time in milliseconds
    double getTimeMs(LARGE_INTEGER start, LARGE_INTEGER end) {
        return (double)(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
    }
    
public:
    Benchmark() {
        QueryPerformanceFrequency(&frequency);
    }
    
    // benchmark with different thread counts
    void benchmarkThreadCounts(int numTasks, int taskIterations) {
        globalLogger.info("=== BENCHMARK: Thread Count Comparison ===");
        std::cout << std::endl;
        
        int threadCounts[] = {1, 2, 4, 8};
        int numTests = 4;
        
        std::cout << "Tasks: " << numTasks << " | Iterations per task: " << taskIterations << "\n" << std::endl;
        
        // allocate task data
        int* taskData = new int[numTasks];
        for (int i = 0; i < numTasks; i++) {
            taskData[i] = taskIterations;
        }
        
        for (int i = 0; i < numTests; i++) {
            int threadCount = threadCounts[i];
            
            char msg[128];
            sprintf_s(msg, "Testing with %d worker thread(s)...", threadCount);
            globalLogger.info(msg);
            
            LARGE_INTEGER start, end;
            QueryPerformanceCounter(&start);
            
            {
                TaskScheduler scheduler(threadCount);
                
                // enqueue all tasks
                for (int j = 0; j < numTasks; j++) {
                    scheduler.enqueueTask(BenchmarkTask, &taskData[j]);
                }
                
                // wait for completion
                Sleep(100); // small delay to start
                while (scheduler.getMetrics().getPendingTasks() > 0) {
                    Sleep(10);
                }
            }
            
            QueryPerformanceCounter(&end);
            double timeMs = getTimeMs(start, end);
            double tasksPerSec = (numTasks * 1000.0) / timeMs;
            
            sprintf_s(msg, "  Time: %.2f ms | Throughput: %.2f tasks/sec", timeMs, tasksPerSec);
            globalLogger.success(msg);
        }
        
        delete[] taskData;
        std::cout << std::endl;
    }
    
    // benchmark with different task counts
    void benchmarkTaskCounts(int threadCount) {
        globalLogger.info("=== BENCHMARK: Task Count Scaling ===");
        std::cout << std::endl;
        
        int taskCounts[] = {10, 50, 100, 500};
        int numTests = 4;
        int taskIterations = 10000;
        
        std::cout << "Worker threads: " << threadCount << " | Iterations per task: " << taskIterations << "\n" << std::endl;
        
        for (int i = 0; i < numTests; i++) {
            int numTasks = taskCounts[i];
            
            // allocate task data
            int* taskData = new int[numTasks];
            for (int j = 0; j < numTasks; j++) {
                taskData[j] = taskIterations;
            }
            
            char msg[128];
            sprintf_s(msg, "Testing with %d tasks...", numTasks);
            globalLogger.info(msg);
            
            LARGE_INTEGER start, end;
            QueryPerformanceCounter(&start);
            
            {
                TaskScheduler scheduler(threadCount);
                
                for (int j = 0; j < numTasks; j++) {
                    scheduler.enqueueTask(BenchmarkTask, &taskData[j]);
                }
                
                Sleep(100);
                while (scheduler.getMetrics().getPendingTasks() > 0) {
                    Sleep(10);
                }
            }
            
            QueryPerformanceCounter(&end);
            double timeMs = getTimeMs(start, end);
            double tasksPerSec = (numTasks * 1000.0) / timeMs;
            
            sprintf_s(msg, "  Time: %.2f ms | Throughput: %.2f tasks/sec", timeMs, tasksPerSec);
            globalLogger.success(msg);
            
            delete[] taskData;
        }
        
        std::cout << std::endl;
    }
    
    // benchmark priority scheduling overhead
    void benchmarkPriorities(int threadCount, int numTasks) {
        globalLogger.info("=== BENCHMARK: Priority vs No Priority ===");
        std::cout << std::endl;
        
        int taskIterations = 10000;
        
        // allocate task data
        int* taskData = new int[numTasks];
        for (int i = 0; i < numTasks; i++) {
            taskData[i] = taskIterations;
        }
        
        // test 1: all same priority (MEDIUM)
        globalLogger.info("Test 1: All tasks MEDIUM priority...");
        
        LARGE_INTEGER start1, end1;
        QueryPerformanceCounter(&start1);
        
        {
            TaskScheduler scheduler(threadCount);
            for (int i = 0; i < numTasks; i++) {
                scheduler.enqueueTask(BenchmarkTask, &taskData[i], MEDIUM);
            }
            Sleep(100);
            while (scheduler.getMetrics().getPendingTasks() > 0) {
                Sleep(10);
            }
        }
        
        QueryPerformanceCounter(&end1);
        double time1 = getTimeMs(start1, end1);
        
        char msg[128];
        sprintf_s(msg, "  Time: %.2f ms", time1);
        globalLogger.success(msg);
        
        // test 2: mixed priorities
        globalLogger.info("Test 2: Mixed priorities (LOW/MED/HIGH/CRITICAL)...");
        
        LARGE_INTEGER start2, end2;
        QueryPerformanceCounter(&start2);
        
        {
            TaskScheduler scheduler(threadCount);
            for (int i = 0; i < numTasks; i++) {
                TaskPriority prio = (TaskPriority)(i % 4); // rotate through priorities
                scheduler.enqueueTask(BenchmarkTask, &taskData[i], prio);
            }
            Sleep(100);
            while (scheduler.getMetrics().getPendingTasks() > 0) {
                Sleep(10);
            }
        }
        
        QueryPerformanceCounter(&end2);
        double time2 = getTimeMs(start2, end2);
        
        sprintf_s(msg, "  Time: %.2f ms", time2);
        globalLogger.success(msg);
        
        double overhead = ((time2 - time1) / time1) * 100.0;
        sprintf_s(msg, "  Priority overhead: %.2f%%", overhead);
        if (overhead < 5.0) {
            globalLogger.success(msg);
        } else {
            globalLogger.warning(msg);
        }
        
        delete[] taskData;
        std::cout << std::endl;
    }
};

#endif