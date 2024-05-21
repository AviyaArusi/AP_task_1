#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

// Function to initialize the queue
void initializeQueue(Queue* q) {
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

// Function to check if the queue is empty
bool isEmpty(Queue* q) {
    return q->front == NULL;
}
char* strdup(const char* str) {
    size_t len = strlen(str) + 1; // Include space for the null terminator
    char* copy = (char*)malloc(len); // Allocate memory for the copy
    if (copy != NULL) {
        strcpy(copy, str); // Copy the string into the allocated memory
    }
    return copy;
}

// Function to dequeue (pop) an element from the queue
char* dequeue(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty!\n");
        return NULL;
    }
    Node* temp = q->rear;
    char* value = strdup(temp->data); 
    // If there's only one element in the queue
    if (q->front == q->rear) {
        q->front = q->rear = NULL;
    } 
    else {
        // Traverse to the second-to-last node
        Node* current = q->front;
        while (current->next != q->rear) {
            current = current->next;
        }
        q->rear = current;
        q->rear->next = NULL;
    }
    q->size--;
    free(temp->data);
    free(temp);
    return value;
}
// Function to enqueue (push) an element into the queue
void enqueue(Queue* q, char value[]) {
    if (!value)
    {
        return;
    }
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation error\n");
        return;
    }

    // Allocate memory for the data part of the Node
    newNode->data = malloc((strlen(value) + 1) * sizeof(char)); // +1 for the null terminator
    if (!newNode->data) {
        printf("Memory allocation error for data\n");
        free(newNode); // Free the newly allocated Node if data allocation fails
        return;
    }

    strcpy(newNode->data, value); // Copy the content of value into newNode->data

    newNode->next = NULL;
    if (q->front) {
        Node* temp = q->front;
        newNode->next = q->front;
        temp->prev = newNode;
    }
    if (q->size == 0) {
        q->rear = newNode;
        newNode->prev = NULL;
    }
    q->front = newNode;
    q->size++;
    if (q->size == 40)
    {
        dequeue(q);
        printf("Size: %d\n", q->size);
    }
}




// Function to display the elements of the queue
void display(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty!\n");
        return;
    }
    Node* current = q->front;
    while (current) {
        printf("%s ", current->data);
        current = current->next;
    }
    printf("\n");
}

// Main function to demonstrate queue operations
// int main() {
//     Queue q;
//     initializeQueue(&q);
//     display(&q);  // Display the queue elements

//     enqueue(&q, "ron");  
//     enqueue(&q, "aviya");  

//     display(&q);  // Display the queue elements

//     return 0;
// }



