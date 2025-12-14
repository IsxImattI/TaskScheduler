#include <iostream>
#include "Queue.h"

int main() {
    Queue<int> q;

    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);

    std::cout << "Queue size: " << q.size() << std::endl;
    std::cout << "Dequeue: " << q.dequeue() << std::endl;
    std::cout << "Dequeue: " << q.dequeue() << std::endl;
    std::cout << "Queue size: " << q.size() << std::endl;

    return 0;
}