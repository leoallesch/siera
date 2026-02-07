#pragma once

#include <stdint.h>

enum {
  FSM_SIGNAL_ENTER = 0,
  FSM_SIGNAL_EXIT,
  FSM_SIGNAL_USER_START,
};
typedef uint8_t fsm_signal_t;

struct fsm_t;

typedef void (*fsm_state_t)(struct fsm_t* fsm, fsm_signal_t signal, const void* data);

typedef struct fsm_t {
  fsm_state_t current_state;
} fsm_t;

void fsm_init(fsm_t* fsm, fsm_state_t initial_state);
void fsm_signal(fsm_t* fsm, fsm_signal_t signal, const void* data);
void fsm_transition(fsm_t* fsm, fsm_state_t state);
