extern "C" {
#include "siera/ds.h"
#include "siera/ds_keys.h"
}
#include <CppUTest/TestHarness.h>
#include <cstring>

/* ── Fake clock ─────────────────────────────────────────────────────────── */

static uint32_t s_now = 0;
static uint32_t fake_get_ticks(siera_hal_timesource_t*)
{
  return s_now;
}
static siera_hal_timesource_t fake_timesource = { fake_get_ticks };

/* ── Mock persist stream ────────────────────────────────────────────────── */

struct PEntry {
  char name[32];
  uint8_t data[8];
  size_t size;
};
static PEntry s_store[32];
static int s_store_n;

static int mock_persist_read(i_siera_ds_t*, siera_dsk_t key, void* buf, size_t size)
{
  const char* n = siera_ds_key_name(key);
  for(int i = 0; i < s_store_n; i++)
    if(strcmp(s_store[i].name, n) == 0 && s_store[i].size == size) {
      memcpy(buf, s_store[i].data, size);
      return 0;
    }
  return -1;
}

struct MockPersistStream {
  i_siera_ds_t interface;
  siera_event_t on_change;
};

static int mock_persist_write(i_siera_ds_t* iface, siera_dsk_t key, const void* buf, size_t size)
{
  const char* n = siera_ds_key_name(key);
  for(int i = 0; i < s_store_n; i++)
    if(strcmp(s_store[i].name, n) == 0) {
      memcpy(s_store[i].data, buf, size);
      s_store[i].size = size;
      siera_ds_on_change_args_t args = { key, buf };
      siera_event_publish(&((MockPersistStream*)iface)->on_change, &args);
      return 0;
    }
  if(s_store_n < 32) {
    strncpy(s_store[s_store_n].name, n, 31);
    memcpy(s_store[s_store_n].data, buf, size);
    s_store[s_store_n].size = size;
    s_store_n++;
    siera_ds_on_change_args_t args = { key, buf };
    siera_event_publish(&((MockPersistStream*)iface)->on_change, &args);
    return 0;
  }
  return -1;
}

static bool mock_persist_contains(i_siera_ds_t*, siera_dsk_t)
{
  return true;
}
static size_t mock_persist_size(i_siera_ds_t*, siera_dsk_t key)
{
  return siera_ds_key_size(key);
}
static siera_event_t* mock_persist_on_change(i_siera_ds_t* iface)
{
  return &((MockPersistStream*)iface)->on_change;
}

static const i_siera_ds_stream_api_t mock_persist_api = {
  mock_persist_read, mock_persist_write, mock_persist_contains, mock_persist_size, mock_persist_on_change
};

/* ── Mock GPIO / ADC streams ────────────────────────────────────────────── */

static bool s_gpio = false;
static uint16_t s_adc = 3300;
static bool s_hw_written = false;

struct MockHwStream {
  i_siera_ds_t interface;
  siera_event_t on_change;
};

static int mock_gpio_read(i_siera_ds_t*, siera_dsk_t, void* buf, size_t)
{
  memcpy(buf, &s_gpio, sizeof(bool));
  return 0;
}
static int mock_gpio_write(i_siera_ds_t* iface, siera_dsk_t key, const void* buf, size_t)
{
  s_hw_written = true;
  siera_ds_on_change_args_t args = { key, buf };
  siera_event_publish(&((MockHwStream*)iface)->on_change, &args);
  return 0;
}
static bool mock_gpio_contains(i_siera_ds_t*, siera_dsk_t)
{
  return true;
}
static size_t mock_gpio_size(i_siera_ds_t*, siera_dsk_t)
{
  return sizeof(bool);
}
static siera_event_t* mock_gpio_on_change(i_siera_ds_t* iface)
{
  return &((MockHwStream*)iface)->on_change;
}
static const i_siera_ds_stream_api_t mock_gpio_api = {
  mock_gpio_read, mock_gpio_write, mock_gpio_contains, mock_gpio_size, mock_gpio_on_change
};

static int mock_adc_read(i_siera_ds_t*, siera_dsk_t, void* buf, size_t)
{
  memcpy(buf, &s_adc, sizeof(uint16_t));
  return 0;
}
static int mock_adc_write(i_siera_ds_t*, siera_dsk_t, const void*, size_t)
{
  return -1;
}
static bool mock_adc_contains(i_siera_ds_t*, siera_dsk_t)
{
  return true;
}
static size_t mock_adc_size(i_siera_ds_t*, siera_dsk_t)
{
  return sizeof(uint16_t);
}
static siera_event_t* mock_adc_on_change(i_siera_ds_t* iface)
{
  return &((MockHwStream*)iface)->on_change;
}
static const i_siera_ds_stream_api_t mock_adc_api = {
  mock_adc_read, mock_adc_write, mock_adc_contains, mock_adc_size, mock_adc_on_change
};

/* ── Test fixture ───────────────────────────────────────────────────────── */

TEST_GROUP(SieraDs)
{
  i_siera_ds_t ds;
  siera_timer_mgr_t timers;

  MockPersistStream persist_s;
  MockHwStream gpio_s;
  MockHwStream adc_s;

  siera_ds_stream_binding_t streams[3] = {
    { SIERA_DS_NVS, &persist_s.interface },
    { SIERA_DS_GPIO, &gpio_s.interface },
    { SIERA_DS_ADC, &adc_s.interface },
  };

  void setup()
  {
    s_now = 0;
    s_store_n = 0;
    s_gpio = false;
    s_adc = 3300;
    s_hw_written = false;

    persist_s.interface.api = &mock_persist_api;
    siera_event_init(&persist_s.on_change);
    gpio_s.interface.api = &mock_gpio_api;
    siera_event_init(&gpio_s.on_change);
    adc_s.interface.api = &mock_adc_api;
    siera_event_init(&adc_s.on_change);

    siera_timer_mgr_init(&timers, &fake_timesource);
    siera_ds_init(&ds, streams, SIERA_NUM_ELEMENTS(streams), &timers, 0);
  }

  void teardown()
  {
    siera_ds_deinit(&ds);
  }
};

/* ── Tests ──────────────────────────────────────────────────────────────── */

TEST(SieraDs, Defaults)
{
  uint8_t h;
  siera_ds_read(&ds, DSK_ALARM_HOUR, &h);
  CHECK_EQUAL(7, h);
}

TEST(SieraDs, Roundtrip)
{
  uint8_t v = 22;
  siera_ds_write(&ds, DSK_ALARM_HOUR, &v);
  uint8_t out;
  siera_ds_read(&ds, DSK_ALARM_HOUR, &out);
  CHECK_EQUAL(22, out);
}

TEST(SieraDs, NoEventOnSameValue)
{
  int count = 0;
  auto counter = [](const void*, void* c) { (*(int*)c)++; };
  siera_event_sub_t sub;
  siera_event_sub_init(&sub, counter, &count);
  siera_ds_subscribe_all(&ds, &sub);

  uint8_t v = 7; // default is 7
  siera_ds_write(&ds, DSK_ALARM_HOUR, &v);
  CHECK_EQUAL(0, count);

  v = 8;
  siera_ds_write(&ds, DSK_ALARM_HOUR, &v);
  CHECK_EQUAL(1, count);
}

TEST(SieraDs, EventCarriesNewData)
{
  uint8_t captured = 0;
  auto on_event = [](const void* args, void* c) {
    const siera_ds_on_change_args_t* e = (const siera_ds_on_change_args_t*)args;
    *(uint8_t*)c = *(const uint8_t*)e->data;
  };
  siera_event_sub_t sub;
  siera_event_sub_init(&sub, on_event, &captured);
  siera_ds_subscribe_all(&ds, &sub);

  uint8_t v = 99;
  siera_ds_write(&ds, DSK_ALARM_HOUR, &v);
  CHECK_EQUAL(99, captured);
}

TEST(SieraDs, PersistWriteThrough)
{
  uint8_t v = 99;
  siera_ds_write(&ds, DSK_BRIGHTNESS, &v);
  siera_ds_deinit(&ds);

  siera_timer_mgr_t tm2;
  siera_timer_mgr_init(&tm2, &fake_timesource);
  i_siera_ds_t ds2;

  siera_ds_init(&ds2, streams, SIERA_NUM_ELEMENTS(streams), &tm2, 0);

  uint8_t out;
  siera_ds_read(&ds2, DSK_BRIGHTNESS, &out);
  CHECK_EQUAL(99, out);
  siera_ds_deinit(&ds2);
}

TEST(SieraDs, BatchedFlush)
{
  siera_ds_deinit(&ds);
  siera_timer_mgr_init(&timers, &fake_timesource);
  siera_ds_init(&ds, streams, SIERA_NUM_ELEMENTS(streams), &timers, 1000);

  uint8_t v = 77;
  siera_ds_write(&ds, DSK_BRIGHTNESS, &v);

  // Not yet flushed at t=500
  s_now = 500;
  siera_timer_tick(&timers);
  {
    i_siera_ds_t ds2;
    siera_timer_mgr_t tm2;
    siera_timer_mgr_init(&tm2, &fake_timesource);

    siera_ds_init(&ds2, streams, SIERA_NUM_ELEMENTS(streams), &tm2, 0);
    uint8_t out;
    siera_ds_read(&ds2, DSK_BRIGHTNESS, &out);
    CHECK_EQUAL(128, out); // default, not yet flushed
    siera_ds_deinit(&ds2);
  }

  // Flushed at t=1000
  s_now = 1000;
  siera_timer_tick(&timers);
  {
    i_siera_ds_t ds2;
    siera_timer_mgr_t tm2;
    siera_timer_mgr_init(&tm2, &fake_timesource);

    siera_ds_init(&ds2, streams, SIERA_NUM_ELEMENTS(streams), &tm2, 0);
    uint8_t out;
    siera_ds_read(&ds2, DSK_BRIGHTNESS, &out);
    CHECK_EQUAL(77, out);
    siera_ds_deinit(&ds2);
  }
}

TEST(SieraDs, ReadonlyRejects)
{
  bool v = true;
  CHECK(siera_ds_write(&ds, DSK_BUTTON_SET, &v) != 0);
}

TEST(SieraDs, HwWriteCallsStream)
{
  bool led = true;
  siera_ds_write(&ds, DSK_LED_STATUS, &led);
  CHECK_TRUE(s_hw_written);
}

TEST(SieraDs, HwReadFresh)
{
  s_adc = 4200;
  uint16_t b;
  siera_ds_read(&ds, DSK_BATTERY_MV, &b);
  CHECK_EQUAL(4200, b);

  s_adc = 3100;
  siera_ds_read(&ds, DSK_BATTERY_MV, &b);
  CHECK_EQUAL(3100, b);
}

TEST(SieraDs, DeinitFlushesRemaining)
{
  siera_ds_deinit(&ds);
  siera_timer_mgr_init(&timers, &fake_timesource);
  siera_ds_init(&ds, streams, SIERA_NUM_ELEMENTS(streams), &timers, 10000);

  uint8_t v = 55;
  siera_ds_write(&ds, DSK_BRIGHTNESS, &v);
  siera_ds_deinit(&ds); // should flush before stopping timer

  i_siera_ds_t ds2;
  siera_timer_mgr_t tm2;
  siera_timer_mgr_init(&tm2, &fake_timesource);

  siera_ds_init(&ds2, streams, SIERA_NUM_ELEMENTS(streams), &tm2, 0);
  uint8_t out;
  siera_ds_read(&ds2, DSK_BRIGHTNESS, &out);
  CHECK_EQUAL(55, out);
  siera_ds_deinit(&ds2);
}
