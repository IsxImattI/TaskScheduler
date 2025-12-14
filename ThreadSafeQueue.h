// ThreadSafeQueue.h
#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include <windows.h>

template<typename T>
class ThreadSafeQueue {
private:
    struct Node {
        T data;
        Node* next;

        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int count;

    // WinAPI synchronization primitives
    CRITICAL_SECTION cs;           // Mutex za zašèito queue
    CONDITION_VARIABLE notEmpty;   // Signal ko queue ni prazen
    bool isShutdown;               // Flag za shutdown

public:
    ThreadSafeQueue() : head(nullptr), tail(nullptr), count(0), isShutdown(false) {
        InitializeCriticalSection(&cs);
        InitializeConditionVariable(&notEmpty);
    }

    ~ThreadSafeQueue() {
        EnterCriticalSection(&cs);

        // Poèisti vse node-e
        while (head != nullptr) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }

        LeaveCriticalSection(&cs);
        DeleteCriticalSection(&cs);
    }

    // Dodaj element (thread-safe)
    void enqueue(const T& value) {
        EnterCriticalSection(&cs);

        Node* newNode = new Node(value);

        if (tail == nullptr) {
            head = tail = newNode;
        }
        else {
            tail->next = newNode;
            tail = newNode;
        }
        count++;

        // Obvesti waiting threads da je nov element
        WakeConditionVariable(&notEmpty);

        LeaveCriticalSection(&cs);
    }

    // Odstrani element (blocking - èaka èe je prazen)
    bool dequeue(T& outValue) {
        EnterCriticalSection(&cs);

        // Èakaj dokler ni queue prazen ALI dokler ni shutdown
        while (head == nullptr && !isShutdown) {
            SleepConditionVariableCS(&notEmpty, &cs, INFINITE);
        }

        // Èe je shutdown in queue prazen, vrni false
        if (isShutdown && head == nullptr) {
            LeaveCriticalSection(&cs);
            return false;
        }

        // Odstrani element
        Node* temp = head;
        outValue = head->data;
        head = head->next;

        if (head == nullptr) {
            tail = nullptr;
        }

        delete temp;
        count--;

        LeaveCriticalSection(&cs);
        return true;
    }

    bool isEmpty() {
        EnterCriticalSection(&cs);
        bool empty = (head == nullptr);
        LeaveCriticalSection(&cs);
        return empty;
    }

    int size() {
        EnterCriticalSection(&cs);
        int sz = count;
        LeaveCriticalSection(&cs);
        return sz;
    }

    // Za graceful shutdown
    void shutdown() {
        EnterCriticalSection(&cs);
        isShutdown = true;
        WakeAllConditionVariable(&notEmpty);  // Zbudi vse waiting threads
        LeaveCriticalSection(&cs);
    }
};

#endif