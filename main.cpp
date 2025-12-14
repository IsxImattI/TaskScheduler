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
    
    int taskData[10];
    for (int i = 0; i < 10; i++) {
        taskData[i] = i;
    }
    
    // enqueue 2 long running tasks (they will block workers)
    globalLogger.info("Enqueueing 2 long-running tasks (3 sec each)...");
    scheduler.enqueueTask(LongRunningTask, &taskData[0], LOW, 0);
    scheduler.enqueueTask(LongRunningTask, &taskData[1], LOW, 1);
    
    Sleep(100);
    
    // enqueue CANCELLABLE tasks
    globalLogger.warning("Enqueueing 5 CANCELLABLE tasks...");
    int cancelId1 = scheduler.enqueueCancellableTask(QuickTask, &taskData[2], MEDIUM);
    int cancelId2 = scheduler.enqueueCancellableTask(QuickTask, &taskData[3], MEDIUM);
    int cancelId3 = scheduler.enqueueCancellableTask(QuickTask, &taskData[4], MEDIUM);
    int cancelId4 = scheduler.enqueueCancellableTask(QuickTask, &taskData[5], MEDIUM);
    int cancelId5 = scheduler.enqueueCancellableTask(QuickTask, &taskData[6], MEDIUM);
    
    sprintf_s(msg, "Cancellable task IDs: %d, %d, %d, %d, %d", 
              cancelId1, cancelId2, cancelId3, cancelId4, cancelId5);
    globalLogger.info(msg);
    
    Sleep(500);
    
    // cancel some tasks BEFORE they execute
    std::cout << std::endl;
    globalLogger.error("CANCELLING tasks 1, 3, and 4...");
    scheduler.cancelTask(cancelId2);  // cancel task 3
    scheduler.cancelTask(cancelId4);  // cancel task 5
    scheduler.cancelTask(cancelId5);  // cancel task 6
    
    std::cout << std::endl;
    globalLogger.info("Expected result:");
    std::cout << "  - Tasks 0, 1 should complete (long running)\n";
    std::cout << "  - Tasks 2, 4 should execute (NOT cancelled)\n";
    std::cout << "  - Tasks 3, 5, 6 should be CANCELLED\n" << std::endl;
    
    std::cout << "=== Execution: ===\n" << std::endl;
    
    // wait for all to complete
    Sleep(8000);
    
    std::cout << std::endl;
    globalLogger.success("Demo completed!");
    
    // final metrics
    std::cout << "\n";
    globalLogger.info("=== FINAL METRICS ===");
    scheduler.getMetrics().printStats();
    
    return 0;
}