#ifndef __CIRCULAR_QUEUE_H__
#define __CIRCULAR_QUEUE_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void *data;             // Array to store the elements.
    size_t head;            // Queue head index.
    size_t tail;            // Queue tail index.
    size_t capacity;        // Queue capacity.
    size_t element_size;    // Size of each element in the queue.
    size_t count;           // Number of elements in the queue.
} circular_queue_t; 

circular_queue_t* circular_queue_init(circular_queue_t *queue, void *data, size_t capacity, size_t element_size);
bool circular_queue_enqueue(circular_queue_t *queue, const void *value);
bool circular_queue_dequeue(circular_queue_t *queue, void *value);
bool circular_queue_is_empty(const circular_queue_t *queue);
bool circular_queue_is_full(const circular_queue_t *queue);
size_t circular_queue_size(const circular_queue_t *queue);
bool circular_queue_peek_head(const circular_queue_t *queue, void *value);
bool circular_queue_peek_tail(const circular_queue_t *queue, void *value);
void circular_queue_clear(circular_queue_t *queue);
void circular_queue_destroy(circular_queue_t **queue);

#endif // __CIRCULAR_QUEUE_H__