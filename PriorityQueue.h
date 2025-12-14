#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <windows.h>

template<typename T>
class PriorityQueue {
private:
    struct Node {
        T data;
        Node* next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    Node* head;
    int count;
    
    CRITICAL_SECTION cs;
    CONDITION_VARIABLE notEmpty;
    bool isShutdown;
    
public:
    PriorityQueue() : head(nullptr), count(0), isShutdown(false) {
        InitializeCriticalSection(&cs);
        InitializeConditionVariable(&notEmpty);
    }
    
    ~PriorityQueue() {
        EnterCriticalSection(&cs);
        
        while (head != nullptr) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
        
        LeaveCriticalSection(&cs);
        DeleteCriticalSection(&cs);
    }
    
    // enqueue - insert based on priority
    void enqueue(const T& value) {
        EnterCriticalSection(&cs);
        
        Node* newNode = new Node(value);
        
        // if the list is empty or new node has higher priority than head
        if (head == nullptr || value.priority > head->data.priority) {
            newNode->next = head;
            head = newNode;
        } else {
            // find the correct position to insert
            Node* current = head;
            while (current->next != nullptr && 
                   current->next->data.priority >= value.priority) {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
        
        count++;
        WakeConditionVariable(&notEmpty);
        
        LeaveCriticalSection(&cs);
    }
    
    // dequeue - remove highest priority item
    bool dequeue(T& outValue) {
        EnterCriticalSection(&cs);
        
        while (head == nullptr && !isShutdown) {
            SleepConditionVariableCS(&notEmpty, &cs, INFINITE);
        }
        
        if (isShutdown && head == nullptr) {
            LeaveCriticalSection(&cs);
            return false;
        }
        
        Node* temp = head;
        outValue = head->data;
        head = head->next;
        
        delete temp;
        count--;
        
        LeaveCriticalSection(&cs);
        return true;
    }
    
    void shutdown() {
        EnterCriticalSection(&cs);
        isShutdown = true;
        WakeAllConditionVariable(&notEmpty);
        LeaveCriticalSection(&cs);
    }
    
    int size() {
        EnterCriticalSection(&cs);
        int sz = count;
        LeaveCriticalSection(&cs);
        return sz;
    }
};

#endif