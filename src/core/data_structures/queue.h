#pragma once

#include "list.h"

#include <stdint.h>
#include <stdbool.h>

typedef list_node_t queue_node_t;

typedef struct {
  list_t list;
  uint16_t size;
} queue_t;

void queue_init(queue_t *queue);
void queue_enqueue(queue_t *queue, queue_node_t *node);
queue_node_t *queue_dequeue(queue_t *queue);
queue_node_t *queue_peek(queue_t *queue);

uint16_t queue_size(queue_t *queue);
bool queue_is_empty(queue_t *queue);