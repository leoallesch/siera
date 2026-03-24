extern "C" {
#include "siera/ds.h"
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

static int mock_persist_read(void*, siera_ds_key_t key, void* buf, size_t size)
{
  const char* n = siera_ds_key_name(key);
  for(int i = 0; i < s_store_n; i++)
    if(strcmp(s_store[i].name, n) == 0 && s_store[i].size == size) {
      memcpy(buf, s_store[i].data, size);
      return 0;
    }
  return -1;
}

static int mock_persist_write(void*, siera_ds_key_t key, const void* buf, size_t size)
{
  const char* n = siera_ds_key_name(key);
  for(int i = 0; i < s_store_n; i++)
    if(strcmp(s_store[i].name, n) == 0) {
      memcpy(s_store[i].data, buf, size);
      s_store[i].size = size;
      return 0;
    }
  if(s_store_n < 32) {
    strncpy(s_store[s_store_n].name, n, 31);
    memcpy(s_store[s_store_n].data, buf, size);
    s_store[s_store_n].size = size;
    s_store_n++;
    return 0;
  }
  return -1;
}

static const siera_ds_stream_api_t mock_persist_api = {
  mock_persist_read, mock_persist_write
};

/* ── Mock GPIO / ADC streams ────────────────────────────────────────────── */

static bool s_gpio = false;
static uint16_t s_adc = 3300;
static bool s_hw_written = false;

static int mock_gpio_read(void*, siera_ds_key_t, void* buf, size_t)
{
  memcpy(buf, &s_gpio, sizeof(bool));
  return 0;
}
static int mock_gpio_write(void*, siera_ds_key_t, const void*, size_t)
{
  s_hw_written = true;
  return 0;
}
static const siera_ds_stream_api_t mock_gpio_api = { mock_gpio_read, mock_gpio_write };

static int mock_adc_read(void*, siera_ds_key_t, void* buf, size_t)
{
  memcpy(buf, &s_adc, sizeof(uint16_t));
  return 0;
}
static const siera_ds_stream_api_t mock_adc_api = { mock_adc_read, nullptr };

/* ── Test fixture ───────────────────────────────────────────────────────── */

TEST_GROUP(SieraDs)
{
  siera_ds_t ds;
  siera_timer_mgr_t timers;

  siera_ds_stream_t persist_s = { &mock_persist_api, nullptr };
  siera_ds_stream_t gpio_s = { &mock_gpio_api, nullptr };
  siera_ds_stream_t adc_s = { &mock_adc_api, nullptr };

  siera_ds_stream_binding_t streams[3] = {
    { SIERA_DS_NVS, &persist_s },
    { SIERA_DS_GPIO, &gpio_s },
    { SIERA_DS_ADC, &adc_s },
  };

  void setup()
  {
    s_now = 0;
    s_store_n = 0;
    s_gpio = false;
    s_adc = 3300;
    s_hw_written = false;
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
  siera_ds_read(&ds, SIERA_DS_KEY_ALARM_HOUR, &h);
  CHECK_EQUAL(7, h);
}

TEST(SieraDs, Roundtrip)
{
  uint8_t v = 22;
  siera_ds_write(&ds, SIERA_DS_KEY_ALARM_HOUR, &v);
  uint8_t out;
  siera_ds_read(&ds, SIERA_DS_KEY_ALARM_HOUR, &out);
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
  siera_ds_write(&ds, SIERA_DS_KEY_ALARM_HOUR, &v);
  CHECK_EQUAL(0, count);

  v = 8;
  siera_ds_write(&ds, SIERA_DS_KEY_ALARM_HOUR, &v);
  CHECK_EQUAL(1, count);
}

TEST(SieraDs, EventCarriesNewData)
{
  uint8_t captured = 0;
  auto on_event = [](const void* args, void* c) {
    const siera_ds_on_change_t* e = (const siera_ds_on_change_t*)args;
    *(uint8_t*)c = *(const uint8_t*)e->data;
  };
  siera_event_sub_t sub;
  siera_event_sub_init(&sub, on_event, &captured);
  siera_ds_subscribe_all(&ds, &sub);

  uint8_t v = 99;
  siera_ds_write(&ds, SIERA_DS_KEY_ALARM_HOUR, &v);
  CHECK_EQUAL(99, captured);
}

TEST(SieraDs, PersistWriteThrough)
{
  uint8_t v = 99;
  siera_ds_write(&ds, SIERA_DS_KEY_BRIGHTNESS, &v);
  siera_ds_deinit(&ds);

  siera_timer_mgr_t tm2;
  siera_timer_mgr_init(&tm2, &fake_timesource);
  siera_ds_t ds2;

  siera_ds_init(&ds2, streams, SIERA_NUM_ELEMENTS(streams), &tm2, 0);

  uint8_t out;
  siera_ds_read(&ds2, SIERA_DS_KEY_BRIGHTNESS, &out);
  CHECK_EQUAL(99, out);
  siera_ds_deinit(&ds2);
}

TEST(SieraDs, BatchedFlush)
{
  siera_ds_deinit(&ds);
  siera_timer_mgr_init(&timers, &fake_timesource);
  siera_ds_init(&ds, streams, SIERA_NUM_ELEMENTS(streams), &timers, 1000);

  uint8_t v = 77;
  siera_ds_write(&ds, SIERA_DS_KEY_BRIGHTNESS, &v);

  // Not yet flushed at t=500
  s_now = 500;
  siera_timer_tick(&timers);
  {
    siera_ds_t ds2;
    siera_timer_mgr_t tm2;
    siera_timer_mgr_init(&tm2, &fake_timesource);

    siera_ds_init(&ds2, streams, SIERA_NUM_ELEMENTS(streams), &tm2, 0);
    uint8_t out;
    siera_ds_read(&ds2, SIERA_DS_KEY_BRIGHTNESS, &out);
    CHECK_EQUAL(128, out); // default, not yet flushed
    siera_ds_deinit(&ds2);
  }

  // Flushed at t=1000
  s_now = 1000;
  siera_timer_tick(&timers);
  {
    siera_ds_t ds2;
    siera_timer_mgr_t tm2;
    siera_timer_mgr_init(&tm2, &fake_timesource);

    siera_ds_init(&ds2, streams, SIERA_NUM_ELEMENTS(streams), &tm2, 0);
    uint8_t out;
    siera_ds_read(&ds2, SIERA_DS_KEY_BRIGHTNESS, &out);
    CHECK_EQUAL(77, out);
    siera_ds_deinit(&ds2);
  }
}

TEST(SieraDs, ReadonlyRejects)
{
  bool v = true;
  CHECK(siera_ds_write(&ds, SIERA_DS_KEY_BUTTON_SET, &v) != 0);
}

TEST(SieraDs, HwWriteCallsStream)
{
  bool led = true;
  siera_ds_write(&ds, SIERA_DS_KEY_LED_STATUS, &led);
  CHECK_TRUE(s_hw_written);
}

TEST(SieraDs, HwReadFresh)
{
  s_adc = 4200;
  uint16_t b;
  siera_ds_read(&ds, SIERA_DS_KEY_BATTERY_MV, &b);
  CHECK_EQUAL(4200, b);

  s_adc = 3100;
  siera_ds_read(&ds, SIERA_DS_KEY_BATTERY_MV, &b);
  CHECK_EQUAL(3100, b);
}

TEST(SieraDs, DeinitFlushesRemaining)
{
  siera_ds_deinit(&ds);
  siera_timer_mgr_init(&timers, &fake_timesource);
  siera_ds_init(&ds, streams, SIERA_NUM_ELEMENTS(streams), &timers, 10000);

  uint8_t v = 55;
  siera_ds_write(&ds, SIERA_DS_KEY_BRIGHTNESS, &v);
  siera_ds_deinit(&ds); // should flush before stopping timer

  siera_ds_t ds2;
  siera_timer_mgr_t tm2;
  siera_timer_mgr_init(&tm2, &fake_timesource);

  siera_ds_init(&ds2, streams, SIERA_NUM_ELEMENTS(streams), &tm2, 0);
  uint8_t out;
  siera_ds_read(&ds2, SIERA_DS_KEY_BRIGHTNESS, &out);
  CHECK_EQUAL(55, out);
  siera_ds_deinit(&ds2);
}
