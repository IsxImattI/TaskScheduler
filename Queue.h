// Queue.h
#ifndef QUEUE_H
#define QUEUE_H

template<typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;

        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int count;

public:
    Queue() : head(nullptr), tail(nullptr), count(0) {}

    ~Queue() {
        while (head != nullptr) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }

    void enqueue(const T& value) {
        Node* newNode = new Node(value);

        if (tail == nullptr) {
            head = tail = newNode;
        }
        else {
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }

    T dequeue() {
        if (head == nullptr) {
			// simple error handling for empty queue
            throw "Queue is empty!";
        }

        Node* temp = head;
        T value = head->data;
        head = head->next;

        if (head == nullptr) {
            tail = nullptr;
        }

        delete temp;
        count--;
        return value;
    }

    bool isEmpty() const {
        return head == nullptr;
    }

    int size() const {
        return count;
    }
};

#endif