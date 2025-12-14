#include <iostream>
#include <windows.h>
#include "TaskScheduler.h"
#include "Logger.h"

// task that returns int - calculates factorial
int CalculateFactorial(void* arg) {
    int n = *(int*)arg;
    
    char msg[128];
    sprintf_s(msg, "Calculating factorial of %d...", n);
    globalLogger.task(msg);
    
    Sleep(1000);  // simulate work
    
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    
    sprintf_s(msg, "Factorial(%d) = %d", n, result);
    globalLogger.success(msg);
    
    return result;
}

// task that returns int - sum of squares
int SumOfSquares(void* arg) {
    int n = *(int*)arg;
    
    char msg[128];
    sprintf_s(msg, "Calculating sum of squares up to %d...", n);
    globalLogger.task(msg);
    
    Sleep(800);  // simulate work
    
    int result = 0;
    for (int i = 1; i <= n; i++) {
        result += i * i;
    }
    
    sprintf_s(msg, "Sum of squares up to %d = %d", n, result);
    globalLogger.success(msg);
    
    return result;
}

// task that returns int - fibonacci
int Fibonacci(void* arg) {
    int n = *(int*)arg;
    
    char msg[128];
    sprintf_s(msg, "Calculating Fibonacci(%d)...", n);
    globalLogger.task(msg);
    
    Sleep(1200);  // simulate work
    
    if (n <= 1) return n;
    
    int a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    
    sprintf_s(msg, "Fibonacci(%d) = %d", n, b);
    globalLogger.success(msg);
    
    return b;
}

int main() {
    globalLogger.info("=== TaskScheduler with Future/Promise Pattern ===");
    
    char msg[256];
    sprintf_s(msg, "Main thread ID: %lu", GetCurrentThreadId());
    globalLogger.info(msg);
    std::cout << std::endl;
    
    // create scheduler with 3 worker threads
    globalLogger.info("Creating scheduler with 3 worker threads...");
    TaskScheduler scheduler(3);
    globalLogger.success("Scheduler created successfully!");
    std::cout << std::endl;
    
    // dynamic allocation for task arguments - no hardcoded limits
    int* args = new int[3];
    args[0] = 5;   // factorial(5)
    args[1] = 10;  // sum of squares up to 10
    args[2] = 15;  // fibonacci(15)
    
    globalLogger.info("=== Enqueueing tasks with return values ===");
    std::cout << std::endl;
    
    // enqueue tasks that return values using Future/Promise pattern
    globalLogger.info("Enqueuing Factorial(5) with HIGH priority...");
    Future<int>* factorialFuture = scheduler.enqueueTaskWithReturn<int>(
        CalculateFactorial, &args[0], HIGH
    );
    
    globalLogger.info("Enqueuing SumOfSquares(10) with MEDIUM priority...");
    Future<int>* sumFuture = scheduler.enqueueTaskWithReturn<int>(
        SumOfSquares, &args[1], MEDIUM
    );
    
    globalLogger.info("Enqueuing Fibonacci(15) with CRITICAL priority...");
    Future<int>* fiboFuture = scheduler.enqueueTaskWithReturn<int>(
        Fibonacci, &args[2], CRITICAL
    );
    
    std::cout << std::endl;
    globalLogger.success("All tasks enqueued! Main thread continues...");
    globalLogger.info("Main thread doing other work while tasks execute asynchronously...");
    std::cout << std::endl;
    
    // main thread can do other work here - non-blocking!
    for (int i = 0; i < 3; i++) {
        sprintf_s(msg, "Main thread doing work... (%d/3)", i+1);
        globalLogger.info(msg);
        Sleep(400);
    }
    
    std::cout << std::endl;
    globalLogger.warning("=== Now waiting for results (blocking until ready) ===");
    std::cout << std::endl;
    
    // get results (blocks until ready) - demonstrates Future.get()
    globalLogger.info("Waiting for factorial result...");
    int factorialResult = factorialFuture->get();
    sprintf_s(msg, "Got result: Factorial(5) = %d", factorialResult);
    globalLogger.success(msg);
    
    globalLogger.info("Waiting for sum of squares result...");
    int sumResult = sumFuture->get();
    sprintf_s(msg, "Got result: Sum of Squares(10) = %d", sumResult);
    globalLogger.success(msg);
    
    globalLogger.info("Waiting for fibonacci result...");
    int fiboResult = fiboFuture->get();
    sprintf_s(msg, "Got result: Fibonacci(15) = %d", fiboResult);
    globalLogger.success(msg);
    
    std::cout << std::endl;
    globalLogger.success("=== All results received! ===");
    
    // cleanup - no memory leaks
    delete factorialFuture;
    delete sumFuture;
    delete fiboFuture;
    delete[] args;
    
    std::cout << std::endl;
    globalLogger.info("=== FINAL METRICS ===");
    scheduler.getMetrics().printStats();
    
    return 0;
}