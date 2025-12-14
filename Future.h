#ifndef FUTURE_H
#define FUTURE_H

#include <windows.h>

// future - holds result of async task
template<typename T>
class Future {
private:
    T* result;
    bool isReady;
    CRITICAL_SECTION cs;
    CONDITION_VARIABLE cv;
    
public:
    Future() : result(nullptr), isReady(false) {
        InitializeCriticalSection(&cs);
        InitializeConditionVariable(&cv);
    }
    
    ~Future() {
        if (result != nullptr) {
            delete result;
        }
        DeleteCriticalSection(&cs);
    }
    
    // set result (called by worker thread)
    void setResult(const T& value) {
        EnterCriticalSection(&cs);
        
        if (result != nullptr) {
            delete result;
        }
        
        result = new T(value);
        isReady = true;
        
        // wake all waiting threads
        WakeAllConditionVariable(&cv);
        
        LeaveCriticalSection(&cs);
    }
    
    // get result (blocking - waits until ready)
    T get() {
        EnterCriticalSection(&cs);
        
        // wait until result is ready
        while (!isReady) {
            SleepConditionVariableCS(&cv, &cs, INFINITE);
        }
        
        T value = *result;
        
        LeaveCriticalSection(&cs);
        
        return value;
    }
    
    // check if result is ready (non-blocking)
    bool ready() {
        EnterCriticalSection(&cs);
        bool r = isReady;
        LeaveCriticalSection(&cs);
        return r;
    }
    
    // wait with timeout (milliseconds)
    bool wait(DWORD timeoutMs) {
        EnterCriticalSection(&cs);
        
        if (isReady) {
            LeaveCriticalSection(&cs);
            return true;
        }
        
        BOOL success = SleepConditionVariableCS(&cv, &cs, timeoutMs);
        bool r = isReady;
        
        LeaveCriticalSection(&cs);
        
        return r;
    }
};

// task wrapper for functions that return values
template<typename T>
struct TaskWithReturn {
    typedef T (*ReturnFunction)(void*);
    
    ReturnFunction function;
    void* argument;
    Future<T>* future;
    
    TaskWithReturn(ReturnFunction func, void* arg, Future<T>* fut)
        : function(func), argument(arg), future(fut) {}
};

// helper function to wrap return task for scheduler
template<typename T>
void ReturnTaskWrapper(void* arg) {
    TaskWithReturn<T>* task = (TaskWithReturn<T>*)arg;
    
    // execute function and get result
    T result = task->function(task->argument);
    
    // set result in future
    task->future->setResult(result);
    
    // cleanup
    delete task;
}

#endif