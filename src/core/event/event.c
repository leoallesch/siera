#include "event.h"
#include "list.h"

void event_init(event_t* event)
{
  list_init(&event->subscribers);
}

void event_subscribe(event_t* event, event_subscription_t* subscription)
{
  list_push(&event->subscribers, &subscription->node);
}

void event_unsubscribe(event_t* event, event_subscription_t* subscription)
{
  list_delete(&event->subscribers, &subscription->node);
}

void event_publish(event_t* event, const void* data)
{
  list_for_each(&event->subscribers, e)
  {
    event_subscription_t* subscription = (event_subscription_t*)e;
    subscription->callback(subscription->context, data);
  }
}
