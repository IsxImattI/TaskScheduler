#include <iostream>
#include <windows.h>
#include "ThreadSafeQueue.h"

// producer thread - adds elements to queue
DWORD WINAPI ProducerThread(LPVOID param) {
    ThreadSafeQueue<int>* queue = (ThreadSafeQueue<int>*)param;

    for (int i = 0; i < 10; i++) {
        queue->enqueue(i);
        std::cout << "Produced: " << i << std::endl;
		Sleep(100);  // simulate work
    }

    return 0;
}

// consumer thread - removes elements from queue
DWORD WINAPI ConsumerThread(LPVOID param) {
    ThreadSafeQueue<int>* queue = (ThreadSafeQueue<int>*)param;

    for (int i = 0; i < 10; i++) {
        int value;
        if (queue->dequeue(value)) {
            std::cout << "Consumed: " << value << std::endl;
        }
    }

    return 0;
}

int main() {
    ThreadSafeQueue<int> queue;

	// create producer and consumer threads
    HANDLE producer = CreateThread(NULL, 0, ProducerThread, &queue, 0, NULL);
    HANDLE consumer = CreateThread(NULL, 0, ConsumerThread, &queue, 0, NULL);

	// wait for threads to finish
    WaitForSingleObject(producer, INFINITE);
    WaitForSingleObject(consumer, INFINITE);

	// cleanup
    CloseHandle(producer);
    CloseHandle(consumer);

    std::cout << "Done!" << std::endl;
    return 0;
}