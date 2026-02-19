#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>

extern "C" {
#include "database.h"
#include "event_subscription.h"
#include "i_datastream.h"
#include "ram_datastream.h"
#include "ram_datastream_utils.h"
#include "utils.h"
}

typedef struct {
  int foo;
  char blah;
} test_struct_t;

#define DATABASE_ENTRIES(ENTRY)    \
  ENTRY(KEY_U8, uint8_t)           \
  ENTRY(KEY_CHAR, char)            \
  ENTRY(KEY_STRUCT, test_struct_t) \
  ENTRY(KEY_U16, uint16_t)         \
  ENTRY(KEY_U32, uint32_t)

DATABASE_ENUM(DATABASE_ENTRIES)
DATABASE_STORAGE(DATABASE_ENTRIES)

static ram_datastream_entry_t entries[] = {
  DATABASE_ENTRIES(DATABASE_EXPAND_AS_ENTRY)
};

static const ram_datastream_config_t ds_config = {
  .entries = entries,
  .count   = NUM_ELEMENTS(entries),
};

static void mock_callback(void* context, const void* data)
{
  mock().actualCall("callback")
        .withPointerParameter("context", context)
        .withConstPointerParameter("data", data);
}

TEST_GROUP(DatabaseTests)
{
  ram_datastream_t ram_ds;
  ram_storage_t    storage;
  s_database_t     database;
  i_datastream_t*  streams[1];

  void setup()
  {
    memset(&storage, 0, sizeof(storage));
    ram_datastream_init(&ram_ds, &ds_config, &storage);
    streams[0] = &ram_ds.interface;
    database_init(&database, streams, NUM_ELEMENTS(streams));
  }

  void teardown()
  {
    mock().clear();
  }
};

// --- Init ---

TEST(DatabaseTests, InitSetsStreamPointer)
{
  POINTERS_EQUAL(streams, database.streams);
}

TEST(DatabaseTests, InitSetsCount)
{
  LONGS_EQUAL(1, database.count);
}

// --- Contains ---

TEST(DatabaseTests, ContainsReturnsTrueForAllValidKeys)
{
  CHECK_TRUE(database_contains(&database, KEY_U8));
  CHECK_TRUE(database_contains(&database, KEY_CHAR));
  CHECK_TRUE(database_contains(&database, KEY_STRUCT));
  CHECK_TRUE(database_contains(&database, KEY_U16));
  CHECK_TRUE(database_contains(&database, KEY_U32));
}

TEST(DatabaseTests, ContainsReturnsFalseForKeyEqualToCount)
{
  CHECK_FALSE(database_contains(&database, ds_config.count));
}

TEST(DatabaseTests, ContainsReturnsFalseForLargeKey)
{
  CHECK_FALSE(database_contains(&database, (datastream_key_t)0xFFFF));
}

// --- Size ---

TEST(DatabaseTests, SizeReturnsCorrectSizeForEachType)
{
  LONGS_EQUAL(sizeof(uint8_t),       database_size(&database, KEY_U8));
  LONGS_EQUAL(sizeof(char),          database_size(&database, KEY_CHAR));
  LONGS_EQUAL(sizeof(test_struct_t), database_size(&database, KEY_STRUCT));
  LONGS_EQUAL(sizeof(uint16_t),      database_size(&database, KEY_U16));
  LONGS_EQUAL(sizeof(uint32_t),      database_size(&database, KEY_U32));
}

TEST(DatabaseTests, SizeReturnsZeroForMissingKey)
{
  LONGS_EQUAL(0, database_size(&database, ds_config.count));
}

// --- Read / Write ---

TEST(DatabaseTests, WriteAndReadU8)
{
  uint8_t write_val = 42;
  database_write(&database, KEY_U8, &write_val);

  uint8_t read_val = 0;
  database_read(&database, KEY_U8, &read_val);
  LONGS_EQUAL(42, read_val);
}

TEST(DatabaseTests, WriteAndReadStruct)
{
  test_struct_t write_val = { .foo = 12345, .blah = 'Z' };
  database_write(&database, KEY_STRUCT, &write_val);

  test_struct_t read_val;
  memset(&read_val, 0, sizeof(read_val));
  database_read(&database, KEY_STRUCT, &read_val);
  LONGS_EQUAL(12345, read_val.foo);
  BYTES_EQUAL('Z', read_val.blah);
}

TEST(DatabaseTests, MultipleKeysAreIndependent)
{
  uint8_t  u8_val  = 0xAA;
  uint32_t u32_val = 0xDEADBEEFUL;

  database_write(&database, KEY_U8,  &u8_val);
  database_write(&database, KEY_U32, &u32_val);

  uint8_t  read_u8  = 0;
  uint32_t read_u32 = 0;
  database_read(&database, KEY_U8,  &read_u8);
  database_read(&database, KEY_U32, &read_u32);

  LONGS_EQUAL(0xAA, read_u8);
  UNSIGNED_LONGS_EQUAL(0xDEADBEEFUL, read_u32);
}

TEST(DatabaseTests, WriteToMissingKeyDoesNothing)
{
  uint8_t val = 55;
  database_write(&database, ds_config.count, &val);
}

TEST(DatabaseTests, ReadFromMissingKeyDoesNotModifyOut)
{
  uint8_t out = 0xCC;
  database_read(&database, ds_config.count, &out);
  BYTES_EQUAL(0xCC, out);
}

// --- Change detection via subscribe_all on stream ---

TEST(DatabaseTests, WritePublishesEventOnChange)
{
  event_subscription_t sub;
  int ctx = 0;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe_all(&ram_ds.interface, &sub);

  mock().expectOneCall("callback")
        .withPointerParameter("context", &ctx)
        .ignoreOtherParameters();

  uint8_t value = 42;
  database_write(&database, KEY_U8, &value);

  mock().checkExpectations();
}

TEST(DatabaseTests, WriteDoesNotPublishWhenValueUnchanged)
{
  event_subscription_t sub;
  int ctx = 0;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe_all(&ram_ds.interface, &sub);

  // Storage is zero-initialised; writing 0 must not trigger event
  uint8_t value = 0;
  database_write(&database, KEY_U8, &value);

  mock().checkExpectations();
}

TEST(DatabaseTests, WritePublishesOnlyOnceForSameValue)
{
  event_subscription_t sub;
  int ctx = 0;
  event_subscription_init(&sub, mock_callback, &ctx);
  datastream_subscribe_all(&ram_ds.interface, &sub);

  mock().expectOneCall("callback")
        .withPointerParameter("context", &ctx)
        .ignoreOtherParameters();

  uint8_t value = 55;
  database_write(&database, KEY_U8, &value);
  database_write(&database, KEY_U8, &value); // same value → no second event

  mock().checkExpectations();
}
