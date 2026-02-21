// test_composite_datastream.cpp

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>
#include <stddef.h>

extern "C" {
#include "composite_datastream.h"
#include "event_subscription.h"
#include "i_datastream.h"
#include "ram_datastream.h"
#include "ram_datastream_utils.h"   // if you have helper utils
#include "utils.h"                  // for NUM_ELEMENTS()
}

// ────────────────────────────────────────────────
// Test data type
// ────────────────────────────────────────────────

typedef struct {
    int32_t foo;
    char    blah;
} test_struct_t;

// ────────────────────────────────────────────────
// Single source of truth: all keys and their types
// ────────────────────────────────────────────────

#define DATABASE_ENTRIES(ENTRY)      \
    ENTRY(KEY_U8,     uint8_t)       \
    ENTRY(KEY_CHAR,   char)          \
    ENTRY(KEY_STRUCT, test_struct_t) \
    ENTRY(KEY_U16,    uint16_t)      \
    ENTRY(KEY_U32,    uint32_t)      \
    ENTRY(KEY_FLOAT,  float)

// ────────────────────────────────────────────────
// Generate enum for keys
// ────────────────────────────────────────────────
  enum {                                  
    DATABASE_ENTRIES(DATABASE_EXPAND_AS_ENUM) 
    KEY_COUNT
  };

// ────────────────────────────────────────────────
// Generate the full RAM storage struct
// ────────────────────────────────────────────────
DATABASE_STORAGE(DATABASE_ENTRIES)

// ────────────────────────────────────────────────
// Full/default entries array (single stream case)
// ────────────────────────────────────────────────
static const ram_datastream_entry_t full_entries[] = {
    DATABASE_ENTRIES(DATABASE_EXPAND_AS_ENTRY)
};

static const ram_datastream_config_t full_config = {
    .entries = (ram_datastream_entry_t*)full_entries,
    .count   = NUM_ELEMENTS(full_entries),
};

// ────────────────────────────────────────────────
// Subsets for multi-stream tests (using enum indices)
// ────────────────────────────────────────────────

// Low keys (first three)
static const ram_datastream_entry_t low_entries[] = {
    full_entries[KEY_U8],
    full_entries[KEY_CHAR],
    full_entries[KEY_STRUCT],
};

static const ram_datastream_config_t low_config = {
    .entries = (ram_datastream_entry_t*)low_entries,
    .count   = NUM_ELEMENTS(low_entries),
};

// High keys (remaining)
static const ram_datastream_entry_t high_entries[] = {
    full_entries[KEY_U16],
    full_entries[KEY_U32],
    full_entries[KEY_FLOAT],
};

static const ram_datastream_config_t high_config = {
    .entries = (ram_datastream_entry_t*)high_entries,
    .count   = NUM_ELEMENTS(high_entries),
};

// ────────────────────────────────────────────────
// Test Group with helpers for single & multi stream
// ────────────────────────────────────────────────

TEST_GROUP(CompositeDatastreamTests)
{
    // Resources
    ram_datastream_t     ram_low;
    ram_datastream_t     ram_high;
    ram_storage_t        storage;           // shared between streams in most cases

    composite_datastream_t composite;
    i_datastream_t*      streams[2]   = {0};
    i_datastream_t*      datastream   = NULL;

    // ── Setup / Teardown ─────────────────────────────────────

    void setup() override
    {
        memset(&storage, 0, sizeof(storage));
        memset(&composite, 0, sizeof(composite));
        datastream = &composite.interface;

        // Default: single full stream
        ram_datastream_init(&ram_low, &full_config, &storage);
        streams[0] = &ram_low.interface;

        composite_datastream_init(&composite, streams, 1);
    }

    void teardown() override
    {
        mock().clear();
    }

    // ── Helpers for different configurations ─────────────────

    void use_single_full_stream()
    {
        ram_datastream_init(&ram_low, &full_config, &storage);
        streams[0] = &ram_low.interface;
        composite_datastream_init(&composite, streams, 1);
    }

    void use_two_streams_shared_storage()
    {
        ram_datastream_init(&ram_low,  &low_config,  &storage);
        ram_datastream_init(&ram_high, &high_config, &storage);

        streams[0] = &ram_low.interface;
        streams[1] = &ram_high.interface;

        composite_datastream_init(&composite, streams, 2);
    }

    void use_two_streams_separate_storage()
    {
        static ram_storage_t storage_low;
        static ram_storage_t storage_high;
        memset(&storage_low,  0, sizeof(storage_low));
        memset(&storage_high, 0, sizeof(storage_high));

        ram_datastream_init(&ram_low,  &low_config,  &storage_low);
        ram_datastream_init(&ram_high, &high_config, &storage_high);

        streams[0] = &ram_low.interface;
        streams[1] = &ram_high.interface;

        composite_datastream_init(&composite, streams, 2);
    }

    // Mock callback
    static void mock_on_change(void* context, const void* data)
    {
        mock().actualCall("on_change")
              .withPointerParameter("context", context)
              .withConstPointerParameter("data", data);
    }
};

// ────────────────────────────────────────────────
// Basic initialization tests
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, Init_SingleStream_SetsCorrectPointersAndCount)
{
    POINTERS_EQUAL(streams[0], composite.streams[0]);
    LONGS_EQUAL(1, composite.count);
}

TEST(CompositeDatastreamTests, Init_TwoStreams_SetsPointersAndCount)
{
    use_two_streams_shared_storage();

    LONGS_EQUAL(2, composite.count);
    POINTERS_EQUAL(&ram_low.interface,  composite.streams[0]);
    POINTERS_EQUAL(&ram_high.interface, composite.streams[1]);
}

// // ────────────────────────────────────────────────
// // Contains & Size delegation
// // ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, Contains_ReturnsTrue_ForKeysInAnyStream)
{
    use_two_streams_shared_storage();

    CHECK_TRUE(datastream_contains(datastream, KEY_U8));
    CHECK_TRUE(datastream_contains(datastream, KEY_STRUCT));
    CHECK_TRUE(datastream_contains(datastream, KEY_U32));
    CHECK_FALSE(datastream_contains(datastream, KEY_COUNT));
}

TEST(CompositeDatastreamTests, Size_ReturnsCorrectSize_OrZero)
{
    use_two_streams_shared_storage();

    LONGS_EQUAL(sizeof(uint8_t),     datastream_size(datastream, KEY_U8));
    LONGS_EQUAL(sizeof(test_struct_t), datastream_size(datastream, KEY_STRUCT));
    LONGS_EQUAL(sizeof(uint32_t),    datastream_size(datastream, KEY_U32));
    LONGS_EQUAL(0,                   datastream_size(datastream, 999));
}

// ────────────────────────────────────────────────
// Read / Write across streams
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, WriteAndRead_WorksAcrossMultipleStreams_SharedStorage)
{
    use_two_streams_shared_storage();

    uint8_t       v8  = 0xAB;
    test_struct_t vst = { .foo = 0x12345678, .blah = '#' };
    uint32_t      v32 = 0xDEADBEEF;

    datastream_write(datastream, KEY_U8,     &v8);
    datastream_write(datastream, KEY_STRUCT, &vst);
    datastream_write(datastream, KEY_U32,    &v32);

    uint8_t       r8  = 0;
    test_struct_t rst = {0};
    uint32_t      r32 = 0;

    datastream_read(datastream, KEY_U8,     &r8);
    datastream_read(datastream, KEY_STRUCT, &rst);
    datastream_read(datastream, KEY_U32,    &r32);

    LONGS_EQUAL(0xAB, r8);
    LONGS_EQUAL(0x12345678, rst.foo);
    BYTES_EQUAL('#', rst.blah);
    UNSIGNED_LONGS_EQUAL(0xDEADBEEF, r32);
}

// ────────────────────────────────────────────────
// Change notification propagation
// ────────────────────────────────────────────────

TEST(CompositeDatastreamTests, WriteDifferentKeysInDifferentStreams_TriggersEventEachTime)
{
    use_two_streams_shared_storage();

    event_subscription_t sub;
    int context = 0xBEEF;
    event_subscription_init(&sub, mock_on_change, &context);

    datastream_subscribe_all(datastream, &sub);

    mock().expectNCalls(3, "on_change")
          .withPointerParameter("context", &context)
          .ignoreOtherParameters();

    uint8_t  v8  = 77;
    uint16_t v16 = 50000;
    uint32_t v32 = 0xCAFEBABE;

    datastream_write(datastream, KEY_U8,  &v8);   // low stream
    datastream_write(datastream, KEY_U16, &v16);  // high stream
    datastream_write(datastream, KEY_U32, &v32);  // high stream

    mock().checkExpectations();
}

TEST(CompositeDatastreamTests, WriteSameValueAgain_DoesNotTriggerEvent)
{
    use_two_streams_shared_storage();

    event_subscription_t sub;
    int context = 123;
    event_subscription_init(&sub, mock_on_change, &context);
    datastream_subscribe_all(datastream, &sub);

    uint8_t val = 200;
    datastream_write(datastream, KEY_U8, &val);   // first write → triggers

    mock().expectNoCall("on_change");

    datastream_write(datastream, KEY_U8, &val);   // same value → no event

    mock().checkExpectations();
}

// Add more tests as needed (null pointers, invalid keys, zero streams, etc.)