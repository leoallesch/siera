#include "list.h"
#include "queue.h"

#include <stddef.h>

void queue_init(queue_t* queue)
{
  list_init(&queue->list);
  queue->size = 0;
}

void queue_enqueue(queue_t* queue, queue_node_t* node)
{
  list_push(&queue->list, node);
  queue->size++;
}

queue_node_t* queue_dequeue(queue_t* queue)
{
  if(queue_is_empty(queue)) {
    return NULL;
  }

  queue_node_t* node = queue->list.head;
  list_delete(&queue->list, node);
  queue->size--;
  return node;
}

queue_node_t* queue_peek(queue_t* queue)
{
  return queue->list.head;
}

uint16_t queue_size(queue_t* queue)
{
  return queue->size;
}

bool queue_is_empty(queue_t* queue)
{
  return queue->size == 0;
}
