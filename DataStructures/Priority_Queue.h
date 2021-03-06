#pragma once
#include <stdbool.h>
#include "Dynamic_Array.h"

// Provides a priority queue using a minheap
typedef struct
{
    DynamicArray *arr;
    bool (*cmp)(element_t, element_t);
    int fifo;
} PriorityQueue;

PriorityQueue *CreatePriorirtyQueue(bool (*cmp)(element_t, element_t));

void PriorityQueuePush(PriorityQueue *pq, element_t element);

void PriorityQueuePop(PriorityQueue *pq);

element_t PriorityQueueGetMin(PriorityQueue *pq);

bool PriorityQueueEmpty(PriorityQueue *pq);

void DestroyPriorityQueue(PriorityQueue *pq);