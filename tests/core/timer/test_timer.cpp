#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "double_timesource.h"
#include "timer.h"
}

static void mock_callback(void* context)
{
  mock().actualCall("callback").withPointerParameter("context", context);
}

TEST_GROUP(TimerTests)
{
  s_timer_controller_t controller;
  double_timesource_t timesource;
  s_timer_t timer;
  s_timer_t timer2;

  void setup()
  {
    double_timesource_init(&timesource);
    timer_controller_init(&controller, &timesource.interface);
  }

  void teardown()
  {
    mock().clear();
  }
};

TEST(TimerTests, init_creates_empty_timer_list)
{
  CHECK_TRUE(list_is_empty(&controller.timers));
}

TEST(TimerTests, init_stores_timesource)
{
  POINTERS_EQUAL(&timesource.interface, controller.timesource);
}

TEST(TimerTests, one_shot_timer_fires_when_expired)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);
  double_timesource_set_ticks(&timesource, 100);

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);

  timer_controller_run(&controller);

  mock().checkExpectations();
}

TEST(TimerTests, one_shot_timer_does_not_fire_before_expiration)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);
  double_timesource_set_ticks(&timesource, 99);

  timer_controller_run(&controller);

  mock().checkExpectations();
}

TEST(TimerTests, one_shot_timer_removed_after_firing)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);
  double_timesource_set_ticks(&timesource, 100);

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);

  timer_controller_run(&controller);

  CHECK_TRUE(list_is_empty(&controller.timers));
}

TEST(TimerTests, repeating_timer_fires_multiple_times)
{
  timer_start_repeating(&timer, &controller, 100, mock_callback, nullptr);

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);
  double_timesource_set_ticks(&timesource, 100);
  timer_controller_run(&controller);
  mock().checkExpectations();

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);
  double_timesource_set_ticks(&timesource, 200);
  timer_controller_run(&controller);
  mock().checkExpectations();

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);
  double_timesource_set_ticks(&timesource, 300);
  timer_controller_run(&controller);
  mock().checkExpectations();
}

TEST(TimerTests, repeating_timer_remains_in_list_after_firing)
{
  timer_start_repeating(&timer, &controller, 100, mock_callback, nullptr);
  double_timesource_set_ticks(&timesource, 100);

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);

  timer_controller_run(&controller);

  CHECK_FALSE(list_is_empty(&controller.timers));
}

TEST(TimerTests, timer_stop_removes_timer)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);

  timer_stop(&timer);

  CHECK_TRUE(list_is_empty(&controller.timers));
}

TEST(TimerTests, stopped_timer_does_not_fire)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);
  timer_stop(&timer);
  double_timesource_set_ticks(&timesource, 100);

  timer_controller_run(&controller);

  mock().checkExpectations();
}

TEST(TimerTests, timer_is_active_returns_true_for_active_timer)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);

  CHECK_TRUE(timer_is_active(&controller, &timer));
}

TEST(TimerTests, timer_is_active_returns_false_for_stopped_timer)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);
  timer_stop(&timer);

  CHECK_FALSE(timer_is_active(&controller, &timer));
}

TEST(TimerTests, timer_is_active_returns_false_for_fired_one_shot)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);
  double_timesource_set_ticks(&timesource, 100);

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);
  timer_controller_run(&controller);

  CHECK_FALSE(timer_is_active(&controller, &timer));
}

TEST(TimerTests, callback_receives_context)
{
  int context = 42;
  timer_start_one_shot(&timer, &controller, 100, mock_callback, &context);
  double_timesource_set_ticks(&timesource, 100);

  mock().expectOneCall("callback").withPointerParameter("context", &context);

  timer_controller_run(&controller);

  mock().checkExpectations();
}

TEST(TimerTests, multiple_timers_fire_independently)
{
  int context1 = 1;
  int context2 = 2;
  timer_start_one_shot(&timer, &controller, 100, mock_callback, &context1);
  timer_start_one_shot(&timer2, &controller, 200, mock_callback, &context2);

  double_timesource_set_ticks(&timesource, 100);
  mock().expectOneCall("callback").withPointerParameter("context", &context1);
  timer_controller_run(&controller);
  mock().checkExpectations();

  double_timesource_set_ticks(&timesource, 200);
  mock().expectOneCall("callback").withPointerParameter("context", &context2);
  timer_controller_run(&controller);
  mock().checkExpectations();
}

TEST(TimerTests, run_returns_ticks_until_next_expiration)
{
  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);

  timesource_ticks_t ticks = timer_controller_run(&controller);

  CHECK_EQUAL(100, ticks);
}

TEST(TimerTests, run_returns_min_ticks_with_multiple_timers)
{
  timer_start_one_shot(&timer, &controller, 200, mock_callback, nullptr);
  timer_start_one_shot(&timer2, &controller, 100, mock_callback, nullptr);

  timesource_ticks_t ticks = timer_controller_run(&controller);

  CHECK_EQUAL(100, ticks);
}

TEST(TimerTests, run_returns_max_when_no_timers)
{
  timesource_ticks_t ticks = timer_controller_run(&controller);

  CHECK_EQUAL(UINT32_MAX, ticks);
}

TEST(TimerTests, repeating_timer_overdue_fires_once_and_reschedules)
{
  // Advance time past two intervals at once; timer fires once and reschedules
  timer_start_repeating(&timer, &controller, 100, mock_callback, nullptr);

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);
  double_timesource_set_ticks(&timesource, 250);
  timer_controller_run(&controller);
  mock().checkExpectations();
}

TEST(TimerTests, run_returns_remaining_ticks_after_repeating_fires)
{
  timer_start_repeating(&timer, &controller, 100, mock_callback, nullptr);

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);
  double_timesource_set_ticks(&timesource, 100);
  timesource_ticks_t ticks = timer_controller_run(&controller);

  // After firing at t=100 with interval 100, next expiry is t=200; ticks_to_next = 100
  CHECK_EQUAL(100, ticks);
  mock().checkExpectations();
}

TEST(TimerTests, advance_ticks_helper_works)
{
  double_timesource_advance_ticks(&timesource, 50);
  double_timesource_advance_ticks(&timesource, 50);

  timer_start_one_shot(&timer, &controller, 100, mock_callback, nullptr);
  double_timesource_set_ticks(&timesource, 200);

  mock().expectOneCall("callback").withPointerParameter("context", (void*)nullptr);
  timer_controller_run(&controller);
  mock().checkExpectations();
}
