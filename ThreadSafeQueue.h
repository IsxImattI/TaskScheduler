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
	CRITICAL_SECTION cs;           // mutex for protecting access
	CONDITION_VARIABLE notEmpty;   // signal for not empty queue
	bool isShutdown;               // flag for shutdown

public:
    ThreadSafeQueue() : head(nullptr), tail(nullptr), count(0), isShutdown(false) {
        InitializeCriticalSection(&cs);
        InitializeConditionVariable(&notEmpty);
    }

    ~ThreadSafeQueue() {
        EnterCriticalSection(&cs);

		// cleanup nodes
        while (head != nullptr) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }

        LeaveCriticalSection(&cs);
        DeleteCriticalSection(&cs);
    }

	// add element
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

		// notify one waiting thread
        WakeConditionVariable(&notEmpty);

        LeaveCriticalSection(&cs);
    }

	// remove element
    bool dequeue(T& outValue) {
        EnterCriticalSection(&cs);

		// wait until not empty or shutdown
        while (head == nullptr && !isShutdown) {
            SleepConditionVariableCS(&notEmpty, &cs, INFINITE);
        }

		// if shutdown and empty, return false
        if (isShutdown && head == nullptr) {
            LeaveCriticalSection(&cs);
            return false;
        }

		// remove from head
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

	// for shutdown
    void shutdown() {
        EnterCriticalSection(&cs);
        isShutdown = true;
		WakeAllConditionVariable(&notEmpty);  // wake all waiting threads
        LeaveCriticalSection(&cs);
    }
};

#endif