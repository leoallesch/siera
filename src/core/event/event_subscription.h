#pragma once

#include "list.h"

typedef void (*event_subscription_callback_t)(void* context, const void* data);

typedef struct {
  list_node_t node;
  event_subscription_callback_t callback;
  void* context;
} event_subscription_t;

void event_subscription_init(event_subscription_t* subscription, event_subscription_callback_t callback, void* context);
