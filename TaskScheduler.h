#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <windows.h>
#include "PriorityQueue.h"

// task priority levels
enum TaskPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

// function pointer type for tasks
typedef void (*TaskFunction)(void*);

// task structure
struct Task {
    TaskFunction function;
    void* argument;
    TaskPriority priority;
    int taskId; // for debugging
    
    Task() : function(nullptr), argument(nullptr), priority(MEDIUM), taskId(-1) {}
    Task(TaskFunction func, void* arg, TaskPriority prio = MEDIUM, int id = -1) 
        : function(func), argument(arg), priority(prio), taskId(id) {}
    
    // comparison operator for priority queue
    bool operator<(const Task& other) const {
        return priority < other.priority; // critical tasks have higher priority
    }
};

class TaskScheduler {
private:
    PriorityQueue<Task> taskQueue;
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
    
    // enqueue with default priority
    void enqueueTask(TaskFunction function, void* argument = nullptr) {
        Task task(function, argument, MEDIUM, -1);
        taskQueue.enqueue(task);
    }

    // enqueue with specific priority
    void enqueueTask(TaskFunction function, void* argument, TaskPriority priority, int taskId = -1) {
        Task task(function, argument, priority, taskId);
        taskQueue.enqueue(task);
    }
};

#endif