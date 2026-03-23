#ifndef SIERA_FSM_H
#define SIERA_FSM_H

#include <stdint.h>

typedef enum {
    SIERA_FSM_SIGNAL_ENTER      = 0,
    SIERA_FSM_SIGNAL_EXIT,
    SIERA_FSM_SIGNAL_USER_START,
} siera_fsm_signal_t;

struct siera_fsm_t;

typedef void (*siera_fsm_state_t)(struct siera_fsm_t *fsm,
                                   siera_fsm_signal_t signal,
                                   const void *data);

typedef struct siera_fsm_t {
    siera_fsm_state_t current_state;
} siera_fsm_t;

void siera_fsm_init(siera_fsm_t *fsm, siera_fsm_state_t initial_state);
void siera_fsm_signal(siera_fsm_t *fsm, siera_fsm_signal_t signal, const void *data);
void siera_fsm_transition(siera_fsm_t *fsm, siera_fsm_state_t state);

#endif
