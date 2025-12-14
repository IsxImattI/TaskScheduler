#include <iostream>
#include <windows.h>
#include "TaskScheduler.h"
#include "Logger.h"

void LongRunningTask(void* arg) {
    int taskId = *(int*)arg;
    
    char msg[128];
    sprintf_s(msg, "Task %d started (will run for 3 seconds)", taskId);
    globalLogger.task(msg);
    
    Sleep(3000);  // simulate long work
    
    sprintf_s(msg, "Task %d completed!", taskId);
    globalLogger.success(msg);
}

void QuickTask(void* arg) {
    int taskId = *(int*)arg;
    
    char msg[128];
    sprintf_s(msg, "Quick task %d executing", taskId);
    globalLogger.task(msg);
    
    Sleep(500);
    
    sprintf_s(msg, "Quick task %d done!", taskId);
    globalLogger.success(msg);
}

int main() {
    globalLogger.info("=== TaskScheduler with Task Cancellation Demo ===");
    
    char msg[128];
    sprintf_s(msg, "Main thread ID: %lu", GetCurrentThreadId());
    globalLogger.info(msg);
    std::cout << std::endl;
    
    // create scheduler with 2 worker threads
    globalLogger.info("Creating scheduler with 2 worker threads...");
    TaskScheduler scheduler(2);
    globalLogger.success("Scheduler created successfully!");
    std::cout << std::endl;
    
    // scenario: enqueue multiple cancellable tasks
    globalLogger.info("=== SCENARIO: Cancel tasks before execution ===");
    std::cout << std::endl;
    
    // DYNAMIC ALLOCATION - no hardcoded limits!
    const int numLongTasks = 2;
    const int numCancellableTasks = 5;
    const int totalTasks = numLongTasks + numCancellableTasks;
    
    // dynamically allocate task data
    int* taskData = new int[totalTasks];
    for (int i = 0; i < totalTasks; i++) {
        taskData[i] = i;
    }
    
    // dynamically allocate array for cancellable task IDs
    int* cancelIds = new int[numCancellableTasks];
    
    // enqueue long running tasks (they will block workers)
    sprintf_s(msg, "Enqueueing %d long-running tasks (3 sec each)...", numLongTasks);
    globalLogger.info(msg);
    
    for (int i = 0; i < numLongTasks; i++) {
        scheduler.enqueueTask(LongRunningTask, &taskData[i], LOW, i);
    }
    
    Sleep(100);
    
    // enqueue CANCELLABLE tasks
    sprintf_s(msg, "Enqueueing %d CANCELLABLE tasks...", numCancellableTasks);
    globalLogger.warning(msg);
    
    for (int i = 0; i < numCancellableTasks; i++) {
        cancelIds[i] = scheduler.enqueueCancellableTask(
            QuickTask, 
            &taskData[numLongTasks + i], 
            MEDIUM
        );
    }
    
    // print all cancellable IDs
    std::cout << "  Cancellable task IDs: ";
    for (int i = 0; i < numCancellableTasks; i++) {
        std::cout << cancelIds[i];
        if (i < numCancellableTasks - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    
    Sleep(500);
    
    // cancel some tasks BEFORE they execute
    std::cout << std::endl;
    globalLogger.error("CANCELLING tasks at indices 1, 3, and 4...");
    
    scheduler.cancelTask(cancelIds[1]);  // cancel second task
    scheduler.cancelTask(cancelIds[3]);  // cancel fourth task
    scheduler.cancelTask(cancelIds[4]);  // cancel fifth task
    
    std::cout << std::endl;
    globalLogger.info("Expected result:");
    std::cout << "  - Long running tasks should complete\n";
    std::cout << "  - Cancellable tasks at indices 0, 2 should execute\n";
    std::cout << "  - Cancellable tasks at indices 1, 3, 4 should be CANCELLED\n" << std::endl;
    
    std::cout << "=== Execution: ===\n" << std::endl;
    
    // wait for all to complete
    Sleep(8000);
    
    std::cout << std::endl;
    globalLogger.success("Demo completed!");
    
    // final metrics
    std::cout << "\n";
    globalLogger.info("=== FINAL METRICS ===");
    scheduler.getMetrics().printStats();
    
    // cleanup - no memory leaks
    delete[] taskData;
    delete[] cancelIds;
    
    return 0;
}