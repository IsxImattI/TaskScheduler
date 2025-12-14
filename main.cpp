#include <iostream>
#include <windows.h>
#include "TaskScheduler.h"

void PrintTask(void* arg) {
    int taskId = *(int*)arg;
    std::cout << "Task " << taskId << " executing on thread " 
              << GetCurrentThreadId() << std::endl;
    Sleep(500);  // simulate work
    std::cout << "Task " << taskId << " completed!" << std::endl;
}

int main() {
    std::cout << "Starting TaskScheduler with 4 worker threads..." << std::endl;
    std::cout << "Main thread ID: " << GetCurrentThreadId() << std::endl << std::endl;
    
    // create scheduler with 4 worker threads
    TaskScheduler scheduler(4);
    
    // enqueue 10 tasks
    int taskIds[10];
    for (int i = 0; i < 10; i++) {
        taskIds[i] = i;
        scheduler.enqueueTask(PrintTask, &taskIds[i]);
        std::cout << "Enqueued task " << i << std::endl;
    }
    
    std::cout << "\nWaiting for tasks to complete...\n" << std::endl;
    
    // wait for tasks to finish
    Sleep(6000);
    
    std::cout << "\nAll done!" << std::endl;
    return 0;
}