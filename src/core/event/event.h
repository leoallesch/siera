#pragma once

#include "list.h"
#include "event_subscription.h"

typedef struct
{
    list_t subscribers;
} event_t;

void event_init(event_t* event);
void event_subscribe(event_t* event, event_subscription_t* subscription);
void event_unsubscribe(event_t* event, event_subscription_t* subscription);
void event_publish(event_t* event, const void* data);