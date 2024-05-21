// queue.h
#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

// Define the Node structure
typedef struct Node {
    char* data;
    struct Node* next;
    struct Node* prev;
} Node;

// Define the Queue structure
typedef struct {
    Node* front;
    Node* rear;
    int size;
} Queue;

// Function to initialize the queue
void initializeQueue(Queue* q);

// Function to check if the queue is empty
bool isEmpty(Queue* q);

// Function to enqueue (push) an element into the queue
void enqueue(Queue* q, char value[]);

// Function to dequeue (pop) an element from the rear of the queue
char* dequeue(Queue* q);

// Function to display the elements of the queue
void display(Queue* q);

// Function prototype for strdup
char* strdup(const char* str);

#endif // QUEUE_H
