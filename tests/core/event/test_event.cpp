#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "event.h"
#include "event_subscription.h"
}

static void mock_callback(void* context, const void* data)
{
  mock().actualCall("callback")
        .withPointerParameter("context", context)
        .withConstPointerParameter("data", data);
}

TEST_GROUP(EventTests)
{
  event_t event;
  event_subscription_t subscription;
  event_subscription_t subscription2;

  void setup()
  {
    event_init(&event);
  }

  void teardown()
  {
    mock().clear();
  }
};

TEST(EventTests, init_creates_empty_subscriber_list)
{
  CHECK_TRUE(list_is_empty(&event.subscribers));
}

TEST(EventTests, subscribe_adds_subscription_to_list)
{
  event_subscription_init(&subscription, mock_callback, nullptr);

  event_subscribe(&event, &subscription);

  CHECK_FALSE(list_is_empty(&event.subscribers));
}

TEST(EventTests, unsubscribe_removes_subscription_from_list)
{
  event_subscription_init(&subscription, mock_callback, nullptr);
  event_subscribe(&event, &subscription);

  event_unsubscribe(&event, &subscription);

  CHECK_TRUE(list_is_empty(&event.subscribers));
}

TEST(EventTests, publish_calls_subscriber_callback)
{
  event_subscription_init(&subscription, mock_callback, nullptr);
  event_subscribe(&event, &subscription);

  mock().expectOneCall("callback")
        .withPointerParameter("context", (void*)nullptr)
        .withConstPointerParameter("data", (const void*)nullptr);

  event_publish(&event, nullptr);

  mock().checkExpectations();
}

TEST(EventTests, publish_passes_context_to_callback)
{
  int context = 42;
  event_subscription_init(&subscription, mock_callback, &context);
  event_subscribe(&event, &subscription);

  mock().expectOneCall("callback")
        .withPointerParameter("context", &context)
        .withConstPointerParameter("data", (const void*)nullptr);

  event_publish(&event, nullptr);

  mock().checkExpectations();
}

TEST(EventTests, publish_passes_data_to_callback)
{
  int data = 123;
  event_subscription_init(&subscription, mock_callback, nullptr);
  event_subscribe(&event, &subscription);

  mock().expectOneCall("callback")
        .withPointerParameter("context", (void*)nullptr)
        .withConstPointerParameter("data", &data);

  event_publish(&event, &data);

  mock().checkExpectations();
}

TEST(EventTests, publish_with_no_subscribers_does_nothing)
{
  event_publish(&event, nullptr);

  mock().checkExpectations();
}

TEST(EventTests, publish_calls_all_subscribers)
{
  event_subscription_init(&subscription, mock_callback, nullptr);
  event_subscription_init(&subscription2, mock_callback, nullptr);
  event_subscribe(&event, &subscription);
  event_subscribe(&event, &subscription2);

  mock().expectNCalls(2, "callback")
        .withPointerParameter("context", (void*)nullptr)
        .withConstPointerParameter("data", (const void*)nullptr);

  event_publish(&event, nullptr);

  mock().checkExpectations();
}

TEST(EventTests, unsubscribed_callback_not_called_on_publish)
{
  event_subscription_init(&subscription, mock_callback, nullptr);
  event_subscribe(&event, &subscription);
  event_unsubscribe(&event, &subscription);

  event_publish(&event, nullptr);

  mock().checkExpectations();
}

TEST(EventTests, multiple_publishes_call_callback_each_time)
{
  event_subscription_init(&subscription, mock_callback, nullptr);
  event_subscribe(&event, &subscription);

  mock().expectNCalls(3, "callback")
        .withPointerParameter("context", (void*)nullptr)
        .withConstPointerParameter("data", (const void*)nullptr);

  event_publish(&event, nullptr);
  event_publish(&event, nullptr);
  event_publish(&event, nullptr);

  mock().checkExpectations();
}

TEST(EventTests, unsubscribe_one_of_multiple_subscribers)
{
  int context1 = 1;
  int context2 = 2;
  event_subscription_init(&subscription, mock_callback, &context1);
  event_subscription_init(&subscription2, mock_callback, &context2);
  event_subscribe(&event, &subscription);
  event_subscribe(&event, &subscription2);

  event_unsubscribe(&event, &subscription);

  mock().expectOneCall("callback")
        .withPointerParameter("context", &context2)
        .withConstPointerParameter("data", (const void*)nullptr);

  event_publish(&event, nullptr);

  mock().checkExpectations();
}
