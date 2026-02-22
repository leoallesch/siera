#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "composite_datastream.h"
#include "event_subscription.h"
#include "i_datastream.h"
#include <stdio.h>
}

#include "double_datastream.hpp"

enum {
    KEY_U8 = 0,
    KEY_CHAR,
    KEY_STRUCT,
    KEY_U16,
    KEY_U32,
    KEY_FLOAT,
    KEY_INVALID = 999
};

static void dummy_callback(void* ctx, const void* data)
{
  // not used in most tests
  (void)ctx;
  (void)data;
}

TEST_GROUP(CompositeDatastreamTests)
{
  composite_datastream_t composite;
  i_datastream_t* datastream = nullptr;

  double_datastream_t streamA;
  double_datastream_t streamB;
  double_datastream_t streamC;

  void setup()
  {
    double_datastream_init(&streamA);
    double_datastream_init(&streamB);
    double_datastream_init(&streamC);

    datastream = &composite.interface;

    // Default: single stream
    i_datastream_t* streams[] = { &streamA.interface };
    composite_datastream_init(&composite, streams, 1);
  
    printf("initted\n");
}

  void teardown()
  {
    mock().checkExpectations();
    mock().clear();
  }

  void use_two_streams()
  {
    i_datastream_t* streams[] = { &streamA.interface, &streamB.interface };
    composite_datastream_init(&composite, streams, 2);
  }

  void use_three_streams()
  {
    i_datastream_t* streams[] = { &streamA.interface, &streamB.interface, &streamC.interface };
    composite_datastream_init(&composite, streams, 3);
  }
};

// ────────────────────────────────────────────────
// Initialization
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, Init_StoresStreamsAndCount_SingleStream)
{
  POINTERS_EQUAL(&streamA.interface, composite.streams[0]);
  LONGS_EQUAL(1, composite.count);
}

TEST(CompositeDatastreamTests, Init_StoresStreamsAndCount_TwoStreams)
{
  use_two_streams();

  LONGS_EQUAL(2, composite.count);
  POINTERS_EQUAL(&streamA.interface, composite.streams[0]);
  POINTERS_EQUAL(&streamB.interface, composite.streams[1]);
}

// ────────────────────────────────────────────────
// contains() – short-circuits on first match
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, Contains_ReturnsFalse_WhenNoStreamHasKey)
{
  use_two_streams();

  double_expect_contains(&streamA, KEY_U8, false);
  double_expect_contains(&streamB, KEY_U8, false);

  CHECK_FALSE(datastream_contains(datastream, KEY_U8));
}

TEST(CompositeDatastreamTests, Contains_ReturnsTrue_WhenSecondStreamHasKey)
{
  use_two_streams();

  double_expect_contains(&streamA, KEY_U16, false);
  double_expect_contains(&streamB, KEY_U16, true);

  CHECK_TRUE(datastream_contains(datastream, KEY_U16));
}

// ────────────────────────────────────────────────
// size()
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, Size_ReturnsZero_WhenKeyNotFound)
{
  double_expect_contains(&streamA, KEY_FLOAT, false);

  LONGS_EQUAL(0, datastream_size(datastream, KEY_FLOAT));
}

TEST(CompositeDatastreamTests, Size_ReturnsCorrectValue_FromMatchingStream)
{
  double_expect_contains(&streamA, KEY_U32, true);
  double_expect_size(&streamA, KEY_U32, sizeof(uint32_t));

  LONGS_EQUAL(sizeof(uint32_t), datastream_size(datastream, KEY_U32));
}

// ────────────────────────────────────────────────
// read()
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, Read_ForwardsToFirstMatchingStream)
{
  use_two_streams();

  uint32_t expected_value = 0xDEADBEEF;

  double_expect_contains(&streamA, KEY_U32, false);
  double_expect_contains(&streamB, KEY_U32, true);
  double_expect_read(&streamB, KEY_U32, &expected_value, sizeof(expected_value));

  uint32_t result = 0;
  datastream_read(datastream, KEY_U32, &result);

  UNSIGNED_LONGS_EQUAL(0xDEADBEEF, result);
}

TEST(CompositeDatastreamTests, Read_DoesNothing_WhenKeyNotFound)
{
  uint8_t original = 0xCC;
  uint8_t buffer = original;

  double_expect_contains(&streamA, 999, false);

  datastream_read(datastream, 999, &buffer);

  BYTES_EQUAL(original, buffer); // unchanged
}

// ────────────────────────────────────────────────
// write()
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, Write_ForwardsToFirstMatchingStreamOnly)
{
  use_two_streams();

  uint16_t value = 60000;

  double_expect_contains(&streamA, KEY_U16, true);
  double_expect_write(&streamA, KEY_U16, &value, sizeof(value));

  // Should NOT be called on streamB
  double_expect_no_calls(&streamB);

  datastream_write(datastream, KEY_U16, &value);
}

TEST(CompositeDatastreamTests, Write_Silent_WhenKeyNotFound)
{
  uint8_t value = 42;

  double_expect_contains(&streamA, 1234, false);

  datastream_write(datastream, 1234, &value);
  // no crash, no call expected
}

// ────────────────────────────────────────────────
// subscribe_all()
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, SubscribeAll_CallsEveryUnderlyingStream)
{
  use_three_streams();

  event_subscription_t sub = {};
  event_subscription_init(&sub, dummy_callback, nullptr);

  double_expect_subscribe_all(&streamA, &sub);
  double_expect_subscribe_all(&streamB, &sub);
  double_expect_subscribe_all(&streamC, &sub);

  datastream_subscribe_all(datastream, &sub);
}

// ────────────────────────────────────────────────
// Edge cases
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, ZeroStreams_InitIsSafe)
{
  composite_datastream_init(&composite, nullptr, 0);

  LONGS_EQUAL(0, composite.count);
  POINTERS_EQUAL(nullptr, composite.streams);

  CHECK_FALSE(datastream_contains(datastream, KEY_U8));
  LONGS_EQUAL(0, datastream_size(datastream, KEY_U8));
}

TEST(CompositeDatastreamTests, NullReadBuffer_DoesNotCrash)
{
  double_expect_contains(&streamA, KEY_CHAR, true);

  datastream_read(datastream, KEY_CHAR, nullptr);
  // should not segfault
}
