#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>

extern "C" {
#include "event_subscription.h"
#include "i_datastream.h"
#include "ram_datastream.h"
#include "ram_datastream_utils.h"
#include "utils.h"
}

// ---------------------------------------------------------------------------
// Schema
// ---------------------------------------------------------------------------

typedef struct {
  int16_t x;
  int16_t y;
} point_t;

#define DS_ENTRIES(ENTRY) \
  ENTRY(DS_U8, uint8_t)   \
  ENTRY(DS_U16, uint16_t) \
  ENTRY(DS_U32, uint32_t) \
  ENTRY(DS_POINT, point_t)

DATABASE_ENUM(DS_ENTRIES)
DATABASE_STORAGE(DS_ENTRIES)

static ram_datastream_entry_t g_entries[] = {
  DS_ENTRIES(DATABASE_EXPAND_AS_ENTRY)
};

static const ram_datastream_config_t g_config = {
  .entries = g_entries,
  .count = NUM_ELEMENTS(g_entries),
};

// ---------------------------------------------------------------------------
// Mock callback
// ---------------------------------------------------------------------------

static void mock_callback(void* context, const void* data)
{
  mock().actualCall("callback").withPointerParameter("context", context).withConstPointerParameter("data", data);
}

// ---------------------------------------------------------------------------
// Test group
// ---------------------------------------------------------------------------

TEST_GROUP(RamDatastreamTests)
{
  ram_datastream_t ds;
  ram_storage_t storage;

  void setup()
  {
    memset(&storage, 0, sizeof(storage));
    ram_datastream_init(&ds, &g_config, &storage);
  }

  void teardown()
  {
    mock().clear();
  }
};

// --- Init ---

TEST(RamDatastreamTests, InitZeroesStorage)
{
  uint8_t val = 0xFF;
  datastream_read(&ds.interface, DS_U8, &val);
  BYTES_EQUAL(0, val);
}

TEST(RamDatastreamTests, InitSetsConfigPointer)
{
  POINTERS_EQUAL(&g_config, ds.config);
}

TEST(RamDatastreamTests, InitSetsStoragePointer)
{
  POINTERS_EQUAL(&storage, ds.storage);
}

// --- Contains ---

TEST(RamDatastreamTests, ContainsReturnsTrueForAllValidKeys)
{
  CHECK_TRUE(datastream_contains(&ds.interface, DS_U8));
  CHECK_TRUE(datastream_contains(&ds.interface, DS_U16));
  CHECK_TRUE(datastream_contains(&ds.interface, DS_U32));
  CHECK_TRUE(datastream_contains(&ds.interface, DS_POINT));
}

TEST(RamDatastreamTests, ContainsReturnsFalseForKeyEqualToCount)
{
  CHECK_FALSE(datastream_contains(&ds.interface, g_config.count));
}

TEST(RamDatastreamTests, ContainsReturnsFalseForLargeKey)
{
  CHECK_FALSE(datastream_contains(&ds.interface, (datastream_key_t)0xFFFF));
}

// --- Size ---

TEST(RamDatastreamTests, SizeReturnsCorrectSizeForEachType)
{
  LONGS_EQUAL(sizeof(uint8_t), datastream_size(&ds.interface, DS_U8));
  LONGS_EQUAL(sizeof(uint16_t), datastream_size(&ds.interface, DS_U16));
  LONGS_EQUAL(sizeof(uint32_t), datastream_size(&ds.interface, DS_U32));
  LONGS_EQUAL(sizeof(point_t), datastream_size(&ds.interface, DS_POINT));
}

TEST(RamDatastreamTests, SizeReturnsZeroForInvalidKey)
{
  LONGS_EQUAL(0, datastream_size(&ds.interface, g_config.count));
}

// --- Read / Write ---

TEST(RamDatastreamTests, WriteAndReadU8)
{
  uint8_t w = 42;
  datastream_write(&ds.interface, DS_U8, &w);

  uint8_t r = 0;
  datastream_read(&ds.interface, DS_U8, &r);
  BYTES_EQUAL(42, r);
}

TEST(RamDatastreamTests, WriteAndReadU32)
{
  uint32_t w = 0xDEADBEEFUL;
  datastream_write(&ds.interface, DS_U32, &w);

  uint32_t r = 0;
  datastream_read(&ds.interface, DS_U32, &r);
  UNSIGNED_LONGS_EQUAL(0xDEADBEEFUL, r);
}

TEST(RamDatastreamTests, WriteAndReadStruct)
{
  point_t w = { .x = -100, .y = 200 };
  datastream_write(&ds.interface, DS_POINT, &w);

  point_t r;
  memset(&r, 0, sizeof(r));
  datastream_read(&ds.interface, DS_POINT, &r);
  LONGS_EQUAL(-100, r.x);
  LONGS_EQUAL(200, r.y);
}

TEST(RamDatastreamTests, MultipleKeysAreStoredIndependently)
{
  uint8_t u8 = 0xAB;
  uint32_t u32 = 0x12345678UL;

  datastream_write(&ds.interface, DS_U8, &u8);
  datastream_write(&ds.interface, DS_U32, &u32);

  uint8_t r8 = 0;
  uint32_t r32 = 0;
  datastream_read(&ds.interface, DS_U8, &r8);
  datastream_read(&ds.interface, DS_U32, &r32);

  BYTES_EQUAL(0xAB, r8);
  UNSIGNED_LONGS_EQUAL(0x12345678UL, r32);
}

TEST(RamDatastreamTests, WriteToInvalidKeyDoesNothing)
{
  uint8_t val = 99;
  datastream_write(&ds.interface, g_config.count, &val);

  uint8_t r = 0;
  datastream_read(&ds.interface, DS_U8, &r);
  BYTES_EQUAL(0, r);
}

TEST(RamDatastreamTests, ReadFromInvalidKeyDoesNotModifyOut)
{
  uint8_t out = 0xCC;
  datastream_read(&ds.interface, g_config.count, &out);
  BYTES_EQUAL(0xCC, out);
}

// --- subscribe_all: change detection ---

TEST(RamDatastreamTests, WritePublishesAllOnChangeOnNewValue)
{
  event_subscription_t sub;
  int ctx = 1;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe_all(&ds.interface, &sub);

  uint8_t val = 7;
  mock().expectOneCall("callback").withPointerParameter("context", &ctx).ignoreOtherParameters();

  datastream_write(&ds.interface, DS_U8, &val);

  mock().checkExpectations();
}

TEST(RamDatastreamTests, WriteDoesNotPublishAllOnChangeWhenValueUnchanged)
{
  event_subscription_t sub;
  int ctx = 2;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe_all(&ds.interface, &sub);

  uint8_t val = 0; // storage already zero
  datastream_write(&ds.interface, DS_U8, &val);

  mock().checkExpectations();
}

TEST(RamDatastreamTests, WritePublishesAllOnChangeOnSecondDistinctValue)
{
  event_subscription_t sub;
  int ctx = 3;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe_all(&ds.interface, &sub);

  uint8_t v1 = 10;
  mock().expectOneCall("callback").withPointerParameter("context", &ctx).ignoreOtherParameters();
  datastream_write(&ds.interface, DS_U8, &v1);
  mock().checkExpectations();
  mock().clear();

  uint8_t v2 = 20;
  mock().expectOneCall("callback").withPointerParameter("context", &ctx).ignoreOtherParameters();
  datastream_write(&ds.interface, DS_U8, &v2);
  mock().checkExpectations();
}

TEST(RamDatastreamTests, MultipleSubscribersAllReceiveEvent)
{
  event_subscription_t sub1, sub2;
  int ctx1 = 10, ctx2 = 20;
  event_subscription_init(&sub1, mock_callback, &ctx1);
  event_subscription_init(&sub2, mock_callback, &ctx2);
  datastream_subscribe_all(&ds.interface, &sub1);
  datastream_subscribe_all(&ds.interface, &sub2);

  uint8_t val = 55;
  mock().expectOneCall("callback").withPointerParameter("context", &ctx1).ignoreOtherParameters();
  mock().expectOneCall("callback").withPointerParameter("context", &ctx2).ignoreOtherParameters();

  datastream_write(&ds.interface, DS_U8, &val);

  mock().checkExpectations();
}

// --- per-key subscribe ---

TEST(RamDatastreamTests, SubscribeFiresOnlyForSpecificKey)
{
  event_subscription_t sub;
  int ctx = 5;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe(&ds.interface, DS_U8, &sub);

  // Writing a different key must NOT fire the DS_U8 subscriber
  uint16_t other = 99;
  datastream_write(&ds.interface, DS_U16, &other);
  mock().checkExpectations();

  // Writing DS_U8 SHOULD fire
  uint8_t val = 42;
  mock().expectOneCall("callback").withPointerParameter("context", &ctx).ignoreOtherParameters();
  datastream_write(&ds.interface, DS_U8, &val);
  mock().checkExpectations();
}

TEST(RamDatastreamTests, SubscribeToInvalidKeyDoesNothing)
{
  event_subscription_t sub;
  int ctx = 0;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe(&ds.interface, g_config.count, &sub);
  // Must not crash
}

// --- unsubscribe ---

TEST(RamDatastreamTests, UnsubscribeStopsAllOnChangeCallbacks)
{
  event_subscription_t sub;
  int ctx = 6;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe_all(&ds.interface, &sub);

  datastream_unsubscribe(&ds.interface, &sub);

  uint8_t val = 77;
  datastream_write(&ds.interface, DS_U8, &val);

  mock().checkExpectations(); // no calls expected
}

TEST(RamDatastreamTests, UnsubscribeStopsKeySpecificCallbacks)
{
  event_subscription_t sub;
  int ctx = 7;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe(&ds.interface, DS_U8, &sub);

  datastream_unsubscribe(&ds.interface, &sub);

  uint8_t val = 88;
  datastream_write(&ds.interface, DS_U8, &val);

  mock().checkExpectations(); // no calls expected
}
