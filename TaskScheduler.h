#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <windows.h>
#include "PriorityQueue.h"
#include "Metrics.h"
#include "Logger.h"
#include "Future.h"

// task priority levels
enum TaskPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

// function pointer type for tasks
typedef void (*TaskFunction)(void*);

// task structure - with cancellation support
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
    
    // cancellation tracking - DYNAMIC LINKED LIST (no limit!)
    struct CancellableTask {
        int taskId;
        volatile bool cancelFlag;
        CancellableTask* next;
        
        CancellableTask(int id) : taskId(id), cancelFlag(false), next(nullptr) {}
    };
    
    CancellableTask* cancellableTasksHead;  // head of linked list
    int nextTaskId;  // auto-increment ID
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
    TaskScheduler(int numThreads) : threadCount(numThreads), isRunning(true), 
                                     cancellableTasksHead(nullptr), nextTaskId(0) {
        workerThreads = new HANDLE[threadCount];
        InitializeCriticalSection(&cancelCs);
        
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
        
        // cleanup cancellable tasks linked list
        EnterCriticalSection(&cancelCs);
        CancellableTask* current = cancellableTasksHead;
        while (current != nullptr) {
            CancellableTask* next = current->next;
            delete current;
            current = next;
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
    
    // enqueue CANCELLABLE task - returns task ID (NO LIMIT!)
    int enqueueCancellableTask(TaskFunction function, void* argument, TaskPriority priority = MEDIUM) {
        EnterCriticalSection(&cancelCs);
        
        // create new cancellable task with auto-increment ID
        int taskId = nextTaskId++;
        CancellableTask* ct = new CancellableTask(taskId);
        
        // add to front of linked list (O(1) insertion)
        ct->next = cancellableTasksHead;
        cancellableTasksHead = ct;
        
        LeaveCriticalSection(&cancelCs);
        
        Task task(function, argument, priority, taskId, &ct->cancelFlag);
        taskQueue.enqueue(task);
        metrics.taskEnqueued();
        
        char msg[128];
        sprintf_s(msg, "Cancellable task %d enqueued", taskId);
        globalLogger.info(msg);
        
        return taskId;
    }
    
    // cancel task by ID - searches linked list
    bool cancelTask(int taskId) {
        EnterCriticalSection(&cancelCs);
        
        CancellableTask* current = cancellableTasksHead;
        bool found = false;
        
        while (current != nullptr) {
            if (current->taskId == taskId) {
                current->cancelFlag = true;
                found = true;
                
                char msg[128];
                sprintf_s(msg, "Task %d marked for cancellation", taskId);
                globalLogger.warning(msg);
                break;
            }
            current = current->next;
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

    // enqueue task that returns a value
    template<typename T>
    Future<T>* enqueueTaskWithReturn(T (*function)(void*), void* argument, TaskPriority priority = MEDIUM) {
        // create future
        Future<T>* future = new Future<T>();
        
        // create wrapper task
        TaskWithReturn<T>* returnTask = new TaskWithReturn<T>(function, argument, future);
        
        // enqueue wrapper
        Task task((TaskFunction)ReturnTaskWrapper<T>, returnTask, priority, -1, nullptr);
        taskQueue.enqueue(task);
        metrics.taskEnqueued();
        
        char msg[128];
        sprintf_s(msg, "Task with return value enqueued");
        globalLogger.info(msg);
        
        return future;
    }
};

#endif