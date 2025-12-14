#include <iostream>
#include <windows.h>
#include "TaskScheduler.h"

void PriorityTask(void* arg) {
    int taskId = *(int*)arg;
    
    std::cout << "[Task " << taskId << "] started on thread " 
              << GetCurrentThreadId() << std::endl;
    Sleep(1000);  // simulate work
    std::cout << "[Task " << taskId << "] completed!" << std::endl;
}

int main() {
    std::cout << "=== TaskScheduler with Priority Queue ===" << std::endl;
    std::cout << "Main thread ID: " << GetCurrentThreadId() << std::endl << std::endl;
    
    // create scheduler with 2 worker threads (for clearer output)
    TaskScheduler scheduler(2);
    
    std::cout << "Enqueueing tasks with different priorities...\n" << std::endl;
    
    // create task IDs
    int taskIds[12];
    for (int i = 0; i < 12; i++) {
        taskIds[i] = i;
    }
    
    // enqueue LOW priority tasks
    std::cout << "Adding LOW priority tasks (0-2)..." << std::endl;
    scheduler.enqueueTask(PriorityTask, &taskIds[0], LOW, 0);
    scheduler.enqueueTask(PriorityTask, &taskIds[1], LOW, 1);
    scheduler.enqueueTask(PriorityTask, &taskIds[2], LOW, 2);
    
    Sleep(100); // small delay
    
    // enqueue MEDIUM priority tasks
    std::cout << "Adding MEDIUM priority tasks (3-5)..." << std::endl;
    scheduler.enqueueTask(PriorityTask, &taskIds[3], MEDIUM, 3);
    scheduler.enqueueTask(PriorityTask, &taskIds[4], MEDIUM, 4);
    scheduler.enqueueTask(PriorityTask, &taskIds[5], MEDIUM, 5);
    
    Sleep(100);
    
    // enqueue HIGH priority tasks
    std::cout << "Adding HIGH priority tasks (6-8)..." << std::endl;
    scheduler.enqueueTask(PriorityTask, &taskIds[6], HIGH, 6);
    scheduler.enqueueTask(PriorityTask, &taskIds[7], HIGH, 7);
    scheduler.enqueueTask(PriorityTask, &taskIds[8], HIGH, 8);
    
    Sleep(100);
    
    // enqueue CRITICAL priority tasks
    std::cout << "Adding CRITICAL priority tasks (9-11)..." << std::endl;
    scheduler.enqueueTask(PriorityTask, &taskIds[9], CRITICAL, 9);
    scheduler.enqueueTask(PriorityTask, &taskIds[10], CRITICAL, 10);
    scheduler.enqueueTask(PriorityTask, &taskIds[11], CRITICAL, 11);
    
    std::cout << "\n=== Expected execution order: ===\n";
    std::cout << "CRITICAL tasks (9-11) should execute FIRST\n";
    std::cout << "HIGH tasks (6-8) should execute SECOND\n";
    std::cout << "MEDIUM tasks (3-5) should execute THIRD\n";
    std::cout << "LOW tasks (0-2) should execute LAST\n" << std::endl;
    
    std::cout << "\n=== Actual execution: ===\n" << std::endl;
    
    // wait for all tasks to complete
    Sleep(8000);
    
    std::cout << "\n=== All tasks completed! ===" << std::endl;
    return 0;
}