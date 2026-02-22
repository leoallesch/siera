#include <cstring>
#include "CppUTestExt/MockSupport.h"
#include "double_datastream.hpp"

static bool double_contains(i_datastream_t* self, datastream_key_t key)
{
  return mock()
    .actualCall("contains")
    .onObject(self)
    .withParameter("key", key)
    .returnBoolValueOrDefault(false);
}

static uint8_t double_size(i_datastream_t* self, datastream_key_t key)
{
  return static_cast<uint8_t>(
    mock()
      .actualCall("size")
      .onObject(self)
      .withParameter("key", key)
      .returnIntValueOrDefault(0));
}

static void double_read(i_datastream_t* self, datastream_key_t key, void* out)
{
  mock()
    .actualCall("read")
    .onObject(self)
    .withParameter("key", key)
    .withOutputParameter("out", out);
}

static void double_write(i_datastream_t* self, datastream_key_t key, const void* data)
{
  mock()
    .actualCall("write")
    .onObject(self)
    .withParameter("key", key)
    .withParameter("data", data); // ← fixed: use withParameter
}

static void double_subscribe(i_datastream_t* self, datastream_key_t key, event_subscription_t* sub)
{
  mock()
    .actualCall("subscribe")
    .onObject(self)
    .withParameter("key", key)
    .withParameter("subscription", sub);
}

static void double_subscribe_all(i_datastream_t* self, event_subscription_t* sub)
{
  mock()
    .actualCall("subscribe_all")
    .onObject(self)
    .withParameter("subscription", sub);
}

static void double_unsubscribe(i_datastream_t* self, event_subscription_t* sub)
{
  mock()
    .actualCall("unsubscribe")
    .onObject(self)
    .withParameter("subscription", sub);
}

void double_datastream_init(double_datastream_t* ds)
{
  std::memset(ds, 0, sizeof(*ds));

  ds->interface = (i_datastream_t){
    .read = double_read,
    .write = double_write,
    .contains = double_contains,
    .size = double_size,
    .subscribe = double_subscribe,
    .subscribe_all = double_subscribe_all,
    .unsubscribe = double_unsubscribe,
  };
}

void double_datastream_reset(double_datastream_t* ds)
{
  (void)ds;
}

// ────────────────────────────────────────────────
// Expectation helpers
// ────────────────────────────────────────────────

void double_expect_contains(double_datastream_t* ds, datastream_key_t key, bool returns)
{
  mock().expectOneCall("contains").onObject(&ds->interface).withParameter("key", key).andReturnValue(returns ? 1 : 0);
}

void double_expect_size(double_datastream_t* ds, datastream_key_t key, uint8_t returns)
{
  mock().expectOneCall("size").onObject(&ds->interface).withParameter("key", key).andReturnValue(static_cast<int>(returns));
}

void double_expect_read(double_datastream_t* ds, datastream_key_t key, const void* return_data, size_t size)
{
  mock().expectOneCall("read").onObject(&ds->interface).withParameter("key", key).withOutputParameterReturning("out", const_cast<void*>(return_data), size);
}

void double_expect_write(double_datastream_t* ds, datastream_key_t key, const void* expected_data, size_t size)
{
  mock().expectOneCall("write").onObject(&ds->interface).withParameter("key", key).withParameter("data", expected_data);
}

void double_expect_subscribe(double_datastream_t* ds, datastream_key_t key, event_subscription_t* sub)
{
  mock().expectOneCall("subscribe").onObject(&ds->interface).withParameter("key", key).withParameter("subscription", sub);
}

void double_expect_subscribe_all(double_datastream_t* ds, event_subscription_t* sub)
{
  mock().expectOneCall("subscribe_all").onObject(&ds->interface).withParameter("subscription", sub);
}

void double_expect_unsubscribe(double_datastream_t* ds, event_subscription_t* sub)
{
  mock().expectOneCall("unsubscribe").onObject(&ds->interface).withParameter("subscription", sub);
}

void double_expect_no_calls(double_datastream_t* ds)
{
  mock().expectNoCall("contains");
  mock().expectNoCall("size");
  mock().expectNoCall("read");
  mock().expectNoCall("write");
  mock().expectNoCall("subscribe");
  mock().expectNoCall("subscribe_all");
  mock().expectNoCall("unsubscribe");
}
