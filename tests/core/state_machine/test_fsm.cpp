#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "fsm.h"
}

static void mock_state(fsm_t* fsm, fsm_signal_t signal, const void* data)
{
  mock().actualCall("state")
        .withPointerParameter("fsm", fsm)
        .withUnsignedIntParameter("signal", signal)
        .withConstPointerParameter("data", data);
}

static void mock_state_a(fsm_t* fsm, fsm_signal_t signal, const void* data)
{
  mock().actualCall("state_a")
        .withPointerParameter("fsm", fsm)
        .withUnsignedIntParameter("signal", signal)
        .withConstPointerParameter("data", data);
}

static void mock_state_b(fsm_t* fsm, fsm_signal_t signal, const void* data)
{
  mock().actualCall("state_b")
        .withPointerParameter("fsm", fsm)
        .withUnsignedIntParameter("signal", signal)
        .withConstPointerParameter("data", data);
}

TEST_GROUP(FsmTests)
{
  fsm_t fsm;

  void setup()
  {
  }

  void teardown()
  {
    mock().clear();
  }
};

TEST(FsmTests, init_sets_initial_state)
{
  mock().expectOneCall("state")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_init(&fsm, mock_state);

  POINTERS_EQUAL(mock_state, fsm.current_state);
  mock().checkExpectations();
}

TEST(FsmTests, init_sends_enter_signal_to_initial_state)
{
  mock().expectOneCall("state")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_init(&fsm, mock_state);

  mock().checkExpectations();
}

TEST(FsmTests, signal_calls_current_state_with_signal)
{
  mock().expectOneCall("state")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_init(&fsm, mock_state);
  mock().clear();

  mock().expectOneCall("state")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_USER_START)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_signal(&fsm, FSM_SIGNAL_USER_START, nullptr);

  mock().checkExpectations();
}

TEST(FsmTests, signal_passes_data_to_state)
{
  mock().expectOneCall("state")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_init(&fsm, mock_state);
  mock().clear();

  int data = 42;
  mock().expectOneCall("state")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_USER_START)
        .withConstPointerParameter("data", &data);

  fsm_signal(&fsm, FSM_SIGNAL_USER_START, &data);

  mock().checkExpectations();
}

TEST(FsmTests, transition_sends_exit_to_old_state)
{
  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_init(&fsm, mock_state_a);
  mock().clear();

  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_EXIT)
        .withConstPointerParameter("data", (const void*)nullptr);
  mock().expectOneCall("state_b")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_transition(&fsm, mock_state_b);

  mock().checkExpectations();
}

TEST(FsmTests, transition_sends_enter_to_new_state)
{
  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_init(&fsm, mock_state_a);
  mock().clear();

  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_EXIT)
        .withConstPointerParameter("data", (const void*)nullptr);
  mock().expectOneCall("state_b")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_transition(&fsm, mock_state_b);

  mock().checkExpectations();
}

TEST(FsmTests, transition_updates_current_state)
{
  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_init(&fsm, mock_state_a);
  mock().clear();

  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_EXIT)
        .withConstPointerParameter("data", (const void*)nullptr);
  mock().expectOneCall("state_b")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_transition(&fsm, mock_state_b);

  POINTERS_EQUAL(mock_state_b, fsm.current_state);
  mock().checkExpectations();
}

TEST(FsmTests, signal_after_transition_goes_to_new_state)
{
  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_init(&fsm, mock_state_a);
  mock().clear();

  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_EXIT)
        .withConstPointerParameter("data", (const void*)nullptr);
  mock().expectOneCall("state_b")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_transition(&fsm, mock_state_b);
  mock().clear();

  mock().expectOneCall("state_b")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_USER_START)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_signal(&fsm, FSM_SIGNAL_USER_START, nullptr);

  mock().checkExpectations();
}

TEST(FsmTests, multiple_transitions)
{
  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_init(&fsm, mock_state_a);
  mock().clear();

  // First transition: A -> B
  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_EXIT)
        .withConstPointerParameter("data", (const void*)nullptr);
  mock().expectOneCall("state_b")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_transition(&fsm, mock_state_b);
  mock().clear();

  // Second transition: B -> A
  mock().expectOneCall("state_b")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_EXIT)
        .withConstPointerParameter("data", (const void*)nullptr);
  mock().expectOneCall("state_a")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_transition(&fsm, mock_state_a);

  POINTERS_EQUAL(mock_state_a, fsm.current_state);
  mock().checkExpectations();
}

TEST(FsmTests, user_defined_signal_values)
{
  mock().expectOneCall("state")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", FSM_SIGNAL_ENTER)
        .withConstPointerParameter("data", (const void*)nullptr);
  fsm_init(&fsm, mock_state);
  mock().clear();

  fsm_signal_t custom_signal = FSM_SIGNAL_USER_START + 5;
  mock().expectOneCall("state")
        .withPointerParameter("fsm", &fsm)
        .withUnsignedIntParameter("signal", custom_signal)
        .withConstPointerParameter("data", (const void*)nullptr);

  fsm_signal(&fsm, custom_signal, nullptr);

  mock().checkExpectations();
}
