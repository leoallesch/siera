extern "C" {
#include "siera/fsm.h"
}
#include <CppUTest/TestHarness.h>

// Call recording for state functions
struct StateCall { siera_fsm_t *fsm; siera_fsm_signal_t signal; const void *data; };
static StateCall s_calls[32];
static int       s_call_count;

static void reset_calls() { s_call_count = 0; }

static void record(siera_fsm_t *fsm, siera_fsm_signal_t signal, const void *data) {
    if (s_call_count < 32)
        s_calls[s_call_count++] = {fsm, signal, data};
}

static void mock_state(siera_fsm_t *fsm, siera_fsm_signal_t signal, const void *data) {
    record(fsm, signal, data);
}
static void mock_state_a(siera_fsm_t *fsm, siera_fsm_signal_t signal, const void *data) {
    record(fsm, signal, data);
}
static void mock_state_b(siera_fsm_t *fsm, siera_fsm_signal_t signal, const void *data) {
    record(fsm, signal, data);
}

TEST_GROUP(SieraFsm) {
    siera_fsm_t fsm;
    void setup() { reset_calls(); }
};

TEST(SieraFsm, InitSetsInitialState) {
    siera_fsm_init(&fsm, mock_state);
    CHECK_EQUAL((void*)mock_state, (void*)fsm.current_state);
    CHECK_EQUAL(1, s_call_count);
    CHECK_EQUAL(SIERA_FSM_SIGNAL_ENTER, s_calls[0].signal);
    CHECK(s_calls[0].data == nullptr);
}

TEST(SieraFsm, SignalCallsCurrentStateWithSignal) {
    siera_fsm_init(&fsm, mock_state);
    reset_calls();

    siera_fsm_signal(&fsm, SIERA_FSM_SIGNAL_USER_START, nullptr);

    CHECK_EQUAL(1, s_call_count);
    CHECK_EQUAL(SIERA_FSM_SIGNAL_USER_START, s_calls[0].signal);
    CHECK(s_calls[0].data == nullptr);
}

TEST(SieraFsm, SignalPassesDataToState) {
    siera_fsm_init(&fsm, mock_state);
    reset_calls();

    int data = 42;
    siera_fsm_signal(&fsm, SIERA_FSM_SIGNAL_USER_START, &data);

    CHECK_EQUAL(1, s_call_count);
    CHECK(s_calls[0].data == &data);
}

TEST(SieraFsm, TransitionSendsExitToOldAndEnterToNew) {
    siera_fsm_init(&fsm, mock_state_a);
    reset_calls();

    siera_fsm_transition(&fsm, mock_state_b);

    CHECK_EQUAL(2, s_call_count);
    CHECK_EQUAL(SIERA_FSM_SIGNAL_EXIT,  s_calls[0].signal);  // state_a exit
    CHECK_EQUAL(SIERA_FSM_SIGNAL_ENTER, s_calls[1].signal);  // state_b enter
}

TEST(SieraFsm, TransitionUpdatesCurrentState) {
    siera_fsm_init(&fsm, mock_state_a);
    siera_fsm_transition(&fsm, mock_state_b);
    CHECK_EQUAL((void*)mock_state_b, (void*)fsm.current_state);
}

TEST(SieraFsm, SignalAfterTransitionGoesToNewState) {
    siera_fsm_init(&fsm, mock_state_a);
    siera_fsm_transition(&fsm, mock_state_b);
    reset_calls();

    siera_fsm_signal(&fsm, SIERA_FSM_SIGNAL_USER_START, nullptr);

    // Only state_b should be called
    CHECK_EQUAL(1, s_call_count);
    CHECK(s_calls[0].fsm == &fsm);
    CHECK_EQUAL(SIERA_FSM_SIGNAL_USER_START, s_calls[0].signal);
}

TEST(SieraFsm, MultipleTransitions) {
    siera_fsm_init(&fsm, mock_state_a);
    siera_fsm_transition(&fsm, mock_state_b);
    siera_fsm_transition(&fsm, mock_state_a);
    CHECK_EQUAL((void*)mock_state_a, (void*)fsm.current_state);
}

TEST(SieraFsm, UserDefinedSignalValues) {
    siera_fsm_init(&fsm, mock_state);
    reset_calls();

    siera_fsm_signal_t custom = (siera_fsm_signal_t)(SIERA_FSM_SIGNAL_USER_START + 5);
    siera_fsm_signal(&fsm, custom, nullptr);

    CHECK_EQUAL(1, s_call_count);
    CHECK_EQUAL(custom, s_calls[0].signal);
}
