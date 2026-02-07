#include "fsm.h"

#include <stddef.h>

void fsm_init(fsm_t* fsm, fsm_state_t initial_state)
{
  fsm->current_state = initial_state;
  fsm_signal(fsm, FSM_SIGNAL_ENTER, NULL);
}

void fsm_signal(fsm_t* fsm, fsm_signal_t signal, const void* data)
{
  fsm->current_state(fsm, signal, data);
}

void fsm_transition(fsm_t* fsm, fsm_state_t state)
{
  fsm_signal(fsm, FSM_SIGNAL_EXIT, NULL);
  fsm->current_state = state;
  fsm_signal(fsm, FSM_SIGNAL_ENTER, NULL);
}
