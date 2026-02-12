#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "database.h"
#include "database_utils.h"
#include "event_subscription.h"
}

typedef struct
{
  int foo;
  char blah;
} test_struct;

#define DATABASE_ENTRIES(ENTRY)  \
  ENTRY(KEY_U8, uint8_t)         \
  ENTRY(KEY_CHAR, char)          \
  ENTRY(KEY_STRUCT, test_struct) \
  ENTRY(KEY_U16, uint16_t)       \
  ENTRY(KEY_U32, uint32_t)

DATABASE_ENUM(DATABASE_ENTRIES)
DATABASE_STORAGE(DATABASE_ENTRIES)

static const s_database_entry_t database_entries[] = {
  DATABASE_ENTRIES(DATABASE_EXPAND_AS_ENTRY)
};

static const s_database_config_t database_config = {
  .entries = database_entries,
  .count = NUM_ELEMENTS(database_entries),
};

static void mock_callback(void* context, const void* data)
{
  mock().actualCall("callback").withPointerParameter("context", context).withConstPointerParameter("data", data);
}

TEST_GROUP(Database)
{
  s_database_t database;
  ram_storage_t storage;

  void setup()
  {
    memset(&storage, 0, sizeof(storage));
    database_init(&database, &database_config, &storage);
  }

  void teardown()
  {
    mock().clear();
  }
};

// Init tests
TEST(Database, InitSetsConfigPointer)
{
  POINTERS_EQUAL(&database_config, database.config);
}

TEST(Database, InitSetsStoragePointer)
{
  POINTERS_EQUAL(&storage, database.storage);
}

// Contains tests
TEST(Database, ContainsReturnsTrueForValidKeys)
{
  CHECK_TRUE(database_contains(&database, KEY_U8));
  CHECK_TRUE(database_contains(&database, KEY_STRUCT));
  CHECK_TRUE(database_contains(&database, KEY_U32));
}

TEST(Database, ContainsReturnsFalseForInvalidKeys)
{
  CHECK_FALSE(database_contains(&database, database_config.count));
  CHECK_FALSE(database_contains(&database, UINT16_MAX));
}

// Size tests
TEST(Database, SizeReturnsCorrectSizeForAllTypes)
{
  LONGS_EQUAL(sizeof(uint8_t), database_size(&database, KEY_U8));
  LONGS_EQUAL(sizeof(char), database_size(&database, KEY_CHAR));
  LONGS_EQUAL(sizeof(test_struct), database_size(&database, KEY_STRUCT));
  LONGS_EQUAL(sizeof(uint16_t), database_size(&database, KEY_U16));
  LONGS_EQUAL(sizeof(uint32_t), database_size(&database, KEY_U32));
}

// Read/Write tests
TEST(Database, WriteAndReadU8)
{
  uint8_t write_val = 42;
  database_write(&database, KEY_U8, &write_val);

  uint8_t read_val = 0;
  database_read(&database, KEY_U8, &read_val);
  LONGS_EQUAL(42, read_val);
}

TEST(Database, WriteAndReadStruct)
{
  test_struct write_val = { .foo = 12345, .blah = 'Z' };
  database_write(&database, KEY_STRUCT, &write_val);

  test_struct read_val = { 0 };
  database_read(&database, KEY_STRUCT, &read_val);
  LONGS_EQUAL(12345, read_val.foo);
  BYTES_EQUAL('Z', read_val.blah);
}

TEST(Database, MultipleKeysAreIndependent)
{
  uint8_t u8_val = 0xAA;
  uint32_t u32_val = 0xDEADBEEF;

  database_write(&database, KEY_U8, &u8_val);
  database_write(&database, KEY_U32, &u32_val);

  uint8_t read_u8 = 0;
  uint32_t read_u32 = 0;
  database_read(&database, KEY_U8, &read_u8);
  database_read(&database, KEY_U32, &read_u32);

  LONGS_EQUAL(0xAA, read_u8);
  UNSIGNED_LONGS_EQUAL(0xDEADBEEF, read_u32);
}

// Change detection tests
TEST(Database, WritePublishesEventOnChange)
{
  event_subscription_t subscription;
  int ctx = 0;
  event_subscription_init(&subscription, mock_callback, &ctx);
  database_subscribe_all(&database, &subscription);

  mock().expectOneCall("callback").withPointerParameter("context", &ctx).ignoreOtherParameters();

  uint8_t value = 42;
  database_write(&database, KEY_U8, &value);

  mock().checkExpectations();
}

TEST(Database, WriteDoesNotPublishWhenValueUnchanged)
{
  event_subscription_t subscription;
  int ctx = 0;
  event_subscription_init(&subscription, mock_callback, &ctx);
  database_subscribe_all(&database, &subscription);

  // Storage is zero-initialized, writing 0 should not trigger event
  uint8_t value = 0;
  database_write(&database, KEY_U8, &value);

  mock().checkExpectations();
}

TEST(Database, WritePublishesOnlyOnceForSameValue)
{
  event_subscription_t subscription;
  int ctx = 0;
  event_subscription_init(&subscription, mock_callback, &ctx);
  database_subscribe_all(&database, &subscription);

  mock().expectOneCall("callback").withPointerParameter("context", &ctx).ignoreOtherParameters();

  uint8_t value = 55;
  database_write(&database, KEY_U8, &value);
  database_write(&database, KEY_U8, &value); // Same value, no event

  mock().checkExpectations();
}
