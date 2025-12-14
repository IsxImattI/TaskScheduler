#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <windows.h>
#include "PriorityQueue.h"
#include "Metrics.h"
#include "Logger.h"

// task priority levels
enum TaskPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

// function pointer type for tasks
typedef void (*TaskFunction)(void*);

// task structure - UPDATED with cancellation support
struct Task {
    TaskFunction function;
    void* argument;
    TaskPriority priority;
    int taskId; // for debugging
    volatile bool* cancelFlag; // pointer to cancellation flag
    
    Task() : function(nullptr), argument(nullptr), priority(MEDIUM), taskId(-1), cancelFlag(nullptr) {}
    
    Task(TaskFunction func, void* arg, TaskPriority prio = MEDIUM, int id = -1, volatile bool* cancel = nullptr) 
        : function(func), argument(arg), priority(prio), taskId(id), cancelFlag(cancel) {}
    
    // check if task should be cancelled
    bool isCancelled() const {
        return cancelFlag != nullptr && *cancelFlag;
    }
    
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
    Metrics metrics;
    
    // cancellation tracking
    struct CancellableTask {
        int taskId;
        volatile bool cancelFlag;
        
        CancellableTask(int id) : taskId(id), cancelFlag(false) {}
    };
    
    // simple array to store cancellable tasks (max 100)
    CancellableTask* cancellableTasks[100];
    int cancellableTaskCount;
    CRITICAL_SECTION cancelCs;
    
    static DWORD WINAPI WorkerThreadFunction(LPVOID param) {
        TaskScheduler* scheduler = (TaskScheduler*)param;
        
        while (true) {
            Task task;
            
            if (!scheduler->taskQueue.dequeue(task)) {
                break;
            }
            
            // check if task was cancelled before execution
            if (task.isCancelled()) {
                char msg[128];
                sprintf_s(msg, "Task %d was CANCELLED before execution", task.taskId);
                globalLogger.warning(msg);
                continue;
            }
            
            if (task.function != nullptr) {
                scheduler->metrics.taskStarted();
                task.function(task.argument);
                scheduler->metrics.taskCompleted();
            }
        }
        
        return 0;
    }
    
public:
    TaskScheduler(int numThreads) : threadCount(numThreads), isRunning(true), cancellableTaskCount(0) {
        workerThreads = new HANDLE[threadCount];
        InitializeCriticalSection(&cancelCs);
        
        // initialize cancellable tasks array
        for (int i = 0; i < 100; i++) {
            cancellableTasks[i] = nullptr;
        }
        
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
        
        // cleanup cancellable tasks
        EnterCriticalSection(&cancelCs);
        for (int i = 0; i < cancellableTaskCount; i++) {
            delete cancellableTasks[i];
        }
        LeaveCriticalSection(&cancelCs);
        DeleteCriticalSection(&cancelCs);
    }
    
    // enqueue with default priority
    void enqueueTask(TaskFunction function, void* argument = nullptr) {
        Task task(function, argument, MEDIUM, -1, nullptr);
        taskQueue.enqueue(task);
        metrics.taskEnqueued();
    }

    // enqueue with specific priority
    void enqueueTask(TaskFunction function, void* argument, TaskPriority priority, int taskId = -1) {
        Task task(function, argument, priority, taskId, nullptr);
        taskQueue.enqueue(task);
        metrics.taskEnqueued();
    }
    
    // enqueue CANCELLABLE task - returns task ID
    int enqueueCancellableTask(TaskFunction function, void* argument, TaskPriority priority = MEDIUM) {
        EnterCriticalSection(&cancelCs);
        
        if (cancellableTaskCount >= 100) {
            LeaveCriticalSection(&cancelCs);
            globalLogger.error("Too many cancellable tasks!");
            return -1;
        }
        
        int taskId = cancellableTaskCount;
        CancellableTask* ct = new CancellableTask(taskId);
        cancellableTasks[cancellableTaskCount++] = ct;
        
        LeaveCriticalSection(&cancelCs);
        
        Task task(function, argument, priority, taskId, &ct->cancelFlag);
        taskQueue.enqueue(task);
        metrics.taskEnqueued();
        
        return taskId;
    }
    
    // cancel task by ID
    bool cancelTask(int taskId) {
        EnterCriticalSection(&cancelCs);
        
        bool found = false;
        for (int i = 0; i < cancellableTaskCount; i++) {
            if (cancellableTasks[i]->taskId == taskId) {
                cancellableTasks[i]->cancelFlag = true;
                found = true;
                
                char msg[128];
                sprintf_s(msg, "Task %d marked for cancellation", taskId);
                globalLogger.warning(msg);
                break;
            }
        }
        
        LeaveCriticalSection(&cancelCs);
        
        if (!found) {
            char msg[128];
            sprintf_s(msg, "Task %d not found for cancellation", taskId);
            globalLogger.error(msg);
        }
        
        return found;
    }
    
    // get metrics
    Metrics& getMetrics() {
        return metrics;
    }
};

#endif