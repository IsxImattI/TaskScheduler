#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <windows.h>
#include "ThreadSafeQueue.h"

// function pointer type for tasks
typedef void (*TaskFunction)(void*);

// task structure
struct Task {
    TaskFunction function;
    void* argument;
    
    Task() : function(nullptr), argument(nullptr) {}
    Task(TaskFunction func, void* arg) : function(func), argument(arg) {}
};

class TaskScheduler {
private:
    ThreadSafeQueue<Task> taskQueue;
    HANDLE* workerThreads;
    int threadCount;
    bool isRunning;
    
    static DWORD WINAPI WorkerThreadFunction(LPVOID param) {
        TaskScheduler* scheduler = (TaskScheduler*)param;
        
        while (true) {
            Task task;
            
            if (!scheduler->taskQueue.dequeue(task)) {
                break;
            }
            
            if (task.function != nullptr) {
                task.function(task.argument);
            }
        }
        
        return 0;
    }
    
public:
    TaskScheduler(int numThreads) : threadCount(numThreads), isRunning(true) {
        workerThreads = new HANDLE[threadCount];
        
        for (int i = 0; i < threadCount; i++) {
            workerThreads[i] = CreateThread(
                NULL, 0, WorkerThreadFunction, this, 0, NULL
            );
        }
    }
    
    ~TaskScheduler() {
        taskQueue.shutdown();
        WaitForMultipleObjects(threadCount, workerThreads, TRUE, INFINITE);
        
        for (int i = 0; i < threadCount; i++) {
            CloseHandle(workerThreads[i]);
        }
        delete[] workerThreads;
    }
    
    void enqueueTask(TaskFunction function, void* argument = nullptr) {
        Task task(function, argument);
        taskQueue.enqueue(task);
    }
};

#endif