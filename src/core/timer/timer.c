#include "timer.h"

void timer_controller_init(s_timer_controller_t* controller, i_timesource_t* timesource)
{
  controller->timesource = timesource;
  controller->current_ticks = timesource->get_ticks(timesource);
  list_init(&controller->timers);
}

timesource_ticks_t timer_controller_run(s_timer_controller_t* controller)
{
  controller->current_ticks = controller->timesource->get_ticks(controller->timesource);
  timesource_ticks_t min_ticks_to_next = UINT32_MAX;

  list_node_t* current = controller->timers.head;
  while(current != NULL) {
    list_node_t* next = current->next;
    s_timer_t* timer = (s_timer_t*)current;

    int32_t ticks_until_expiration = (int32_t)(timer->next_expiration_ticks - controller->current_ticks);

    if(ticks_until_expiration <= 0) {
      if(timer->repeating) {
        timer->next_expiration_ticks += timer->interval_ticks;
        timer->callback(timer->context);

        int32_t new_ticks_until = (int32_t)(timer->next_expiration_ticks - controller->current_ticks);
        if(new_ticks_until > 0 && (timesource_ticks_t)new_ticks_until < min_ticks_to_next) {
          min_ticks_to_next = (timesource_ticks_t)new_ticks_until;
        }
      }
      else {
        list_delete(&controller->timers, current);
        timer->callback(timer->context);
      }
    }
    else {
      if((timesource_ticks_t)ticks_until_expiration < min_ticks_to_next) {
        min_ticks_to_next = (timesource_ticks_t)ticks_until_expiration;
      }
    }

    current = next;
  }

  return min_ticks_to_next;
}

static void timer_start(s_timer_t* timer, s_timer_controller_t* controller, timesource_ticks_t interval_ticks, timer_callback_t callback, void* context, bool repeating)
{
  timer->controller = controller;
  timer->callback = callback;
  timer->context = context;
  timer->interval_ticks = interval_ticks;
  timer->next_expiration_ticks = controller->current_ticks + interval_ticks;
  timer->repeating = repeating;
  list_push(&controller->timers, &timer->node);
}

void timer_start_one_shot(s_timer_t* timer, s_timer_controller_t* controller, timesource_ticks_t interval_ticks, timer_callback_t callback, void* context)
{
  timer_start(timer, controller, interval_ticks, callback, context, false);
}

void timer_start_repeating(s_timer_t* timer, s_timer_controller_t* controller, timesource_ticks_t interval_ticks, timer_callback_t callback, void* context)
{
  timer_start(timer, controller, interval_ticks, callback, context, true);
}

void timer_stop(s_timer_t* timer)
{
  list_delete(&timer->controller->timers, &timer->node);
}

bool timer_is_active(s_timer_controller_t* controller, s_timer_t* timer)
{
  list_for_each(&controller->timers, node)
  {
    if(node == &timer->node) {
      return true;
    }
  }
  return false;
}
