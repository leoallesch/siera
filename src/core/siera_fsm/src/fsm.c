#include "siera/fsm.h"

#include <stddef.h>

void siera_fsm_init(siera_fsm_t *fsm, siera_fsm_state_t initial_state) {
    fsm->current_state = initial_state;
    siera_fsm_signal(fsm, SIERA_FSM_SIGNAL_ENTER, NULL);
}

void siera_fsm_signal(siera_fsm_t *fsm, siera_fsm_signal_t signal, const void *data) {
    fsm->current_state(fsm, signal, data);
}

void siera_fsm_transition(siera_fsm_t *fsm, siera_fsm_state_t state) {
    siera_fsm_signal(fsm, SIERA_FSM_SIGNAL_EXIT, NULL);
    fsm->current_state = state;
    siera_fsm_signal(fsm, SIERA_FSM_SIGNAL_ENTER, NULL);
}
