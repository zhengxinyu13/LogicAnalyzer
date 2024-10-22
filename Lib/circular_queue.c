#include "circular_queue.h"

/**
 * @brief Initialization of a circular queue.
 * @param param1 (circular_queue_t *) Points to the circular queue.
 * @param param2 (void *) Points to the data to be stored in the queue.
 * @param param3 (size_t) The capacity of the queue.
 * @param param4 (size_t) The size of each element in the queue.
 * @return Returns a pointer to the initialized circular queue.
 * @throws If something goes wrong, it returns NULL.
 */
circular_queue_t* circular_queue_init(circular_queue_t *queue, void *data, size_t capacity, size_t element_size)
{
    if (!capacity || !element_size)
        return NULL;

    if (NULL == queue) {
        queue = (circular_queue_t *)malloc(sizeof(circular_queue_t));
        if (NULL == queue)
            return NULL;
    }

    if (NULL == data) {
        queue->data = malloc(capacity * element_size);
        if (NULL == queue->data) {
            free(queue);
            return NULL;
        }
    } else {
        queue->data = data;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->capacity = capacity;
    queue->element_size = element_size;
    queue->count = 0;

    return queue;
}

/**
 * @brief Enqueues an element into the circular queue.
 * @param param1 (circular_queue_t *) Points to the circular queue.
 * @param param2 (const void *) Points to the value to be enqueued.
 * @return Returns true if the enqueue operation is successful, otherwise false.
 * @throws If the queue is full or if the parameters are invalid, it returns false.
 */
bool circular_queue_enqueue(circular_queue_t *queue, const void *value)
{
    if (circular_queue_is_full(queue) || NULL == value || NULL == queue)
        return false;

    char *base = (char *)queue->data;
    char *tail_pos = base + (queue->tail * queue->element_size);
    memcpy(tail_pos, value, queue->element_size);

    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    return true;
}

/**
 * @brief Dequeues an element from the circular queue.
 * @param param1 (circular_queue_t *) Points to the circular queue.
 * @param param2 (void *) Points to the location where the dequeued value will be stored.
 * @return Returns true if the dequeue operation is successful, otherwise false.
 * @throws If the queue is empty or if the parameters are invalid, it returns false.
 */
bool circular_queue_dequeue(circular_queue_t *queue, void *value)
{
    if (circular_queue_is_empty(queue) || NULL == value || NULL == queue)
        return false;

    char *base = (char *)queue->data;
    char *head_pos = base + (queue->head * queue->element_size);
    memcpy(value, head_pos, queue->element_size);

    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    return true;
}

/**
 * @brief Checks if the circular queue is empty.
 * @param param1 (const circular_queue_t *) Points to the circular queue.
 * @return Returns true if the queue is empty, otherwise false.
 * @throws If the parameter is invalid, it returns false.
 */
bool circular_queue_is_empty(const circular_queue_t *queue)
{
    if (NULL == queue)
        return false;
    else
        return queue->count == 0;
}

/**
 * @brief Checks if the circular queue is full.
 * @param param1 (const circular_queue_t *) Points to the circular queue.
 * @return Returns true if the queue is full, otherwise false.
 * @throws If the parameter is invalid, it returns false.
 */
bool circular_queue_is_full(const circular_queue_t *queue)
{
    if (NULL == queue)
        return false;
    else
        return queue->count == queue->capacity;
}

/**
 * @brief Gets the current number of elements in the circular queue.
 * @param param1 (const circular_queue_t *) Points to the circular queue.
 * @return Returns the current number of elements in the queue.
 * @throws If the parameter is invalid, it returns -1.
 */
size_t circular_queue_size(const circular_queue_t *queue)
{
    if (NULL == queue)
        return (size_t)-1;
    else
        return queue->count;
}

/**
 * @brief Peeks at the front element of the circular queue.
 * @param param1 (const circular_queue_t *) Points to the circular queue.
 * @param param2 (void *) Points to the location where the front element will be stored.
 * @return Returns true if the peek operation is successful, otherwise false.
 * @throws If the queue is empty or if the parameters are invalid, it returns false.
 */
bool circular_queue_peek_head(const circular_queue_t *queue, void *value)
{
    if (circular_queue_is_empty(queue) || NULL == value || NULL == queue)
        return false;

    char *base = (char *)queue->data;
    char *head_pos = base + (queue->head * queue->element_size);
    memcpy(value, head_pos, queue->element_size);
    return true;
}

/**
 * @brief Peeks at the rear element of the circular queue.
 * @param param1 (const circular_queue_t *) Points to the circular queue.
 * @param param2 (void *) Points to the location where the rear element will be stored.
 * @return Returns true if the peek operation is successful, otherwise false.
 * @throws If the queue is empty or if the parameters are invalid, it returns false.
 */
bool circular_queue_peek_tail(const circular_queue_t *queue, void *value)
{
    if (circular_queue_is_empty(queue) || NULL == value || NULL == queue)
        return false;

    char *base = (char *)queue->data;
    char *tail_pos = base + ((queue->tail - 1 + queue->capacity) % queue->capacity) * queue->element_size;
    memcpy(value, tail_pos, queue->element_size);
    return true;
}

/**
 * @brief Clears the circular queue.
 * @param param1 (circular_queue_t *) Points to the circular queue.
 * @return None.
 */
void circular_queue_clear(circular_queue_t *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
	memcpy(queue->data, "\0", queue->capacity * queue->element_size);
	queue->capacity = 0;
}

/**
 * @brief Destroys the circular queue and frees allocated memory.
 * @param param1 (circular_queue_t **) Points to the pointer to the circular queue.
 * @return None.
 */
void circular_queue_destroy(circular_queue_t **queue)
{
    if (queue == NULL || *queue == NULL)
        return;

    if ((*queue)->data != NULL) {
        free((*queue)->data);
        (*queue)->data = NULL;
    }

    free(*queue);
    *queue = NULL;
}