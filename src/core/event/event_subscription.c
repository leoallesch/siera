#include "event_subscription.h"

void event_subscription_init(event_subscription_t* subscription, event_subscription_callback_t callback, void* context)
{
  subscription->callback = callback;
  subscription->context = context;
}
