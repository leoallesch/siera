extern "C" {
#include "siera/timer.h"
}
#include <CppUTest/TestHarness.h>

// Fake timesource using siera_timesource_t interface
static uint32_t s_now = 0;

static uint32_t fake_get_ticks(siera_hal_timesource_t *) { return s_now; }
static siera_hal_timesource_t fake_timesource = { fake_get_ticks };

static int s_fires = 0;
static void counter(void *) { s_fires++; }

TEST_GROUP(SieraTimer) {
    siera_timer_mgr_t mgr;
    siera_timer_t t;

    void setup() {
        s_now   = 0;
        s_fires = 0;
        siera_timer_mgr_init(&mgr, &fake_timesource);
    }
};

TEST(SieraTimer, Oneshot) {
    siera_timer_start(&mgr, &t, counter, nullptr, 100, false);
    s_now = 50;  siera_timer_tick(&mgr); CHECK_EQUAL(0, s_fires);
    s_now = 100; siera_timer_tick(&mgr); CHECK_EQUAL(1, s_fires);
    CHECK_FALSE(siera_timer_is_active(&t));
}

TEST(SieraTimer, Repeating) {
    siera_timer_start(&mgr, &t, counter, nullptr, 100, true);
    s_now = 100; siera_timer_tick(&mgr); CHECK_EQUAL(1, s_fires);
    s_now = 200; siera_timer_tick(&mgr); CHECK_EQUAL(2, s_fires);
    siera_timer_stop(&mgr, &t);
    s_now = 300; siera_timer_tick(&mgr); CHECK_EQUAL(2, s_fires);
}

TEST(SieraTimer, DriftCorrection) {
    siera_timer_start(&mgr, &t, counter, nullptr, 100, true);
    s_now = 105; siera_timer_tick(&mgr);
    // Expiry should advance from 100, not from 105
    CHECK_EQUAL(200u, t.expiry_ms);
}

TEST(SieraTimer, StopIdempotent) {
    siera_timer_start(&mgr, &t, counter, nullptr, 100, true);
    siera_timer_stop(&mgr, &t);
    siera_timer_stop(&mgr, &t); // second stop should not crash
    s_now = 200; siera_timer_tick(&mgr);
    CHECK_EQUAL(0, s_fires);
}

TEST(SieraTimer, Restart) {
    siera_timer_start(&mgr, &t, counter, nullptr, 100, false);
    s_now = 80; siera_timer_restart(&mgr, &t);
    s_now = 100; siera_timer_tick(&mgr); CHECK_EQUAL(0, s_fires);
    s_now = 180; siera_timer_tick(&mgr); CHECK_EQUAL(1, s_fires);
}

static siera_timer_mgr_t *g_mgr_ptr;
static siera_timer_t       g_self_timer;
static int                 g_self_restarts;

static void self_restart_cb(void *) {
    g_self_restarts++;
    siera_timer_start(g_mgr_ptr, &g_self_timer, self_restart_cb, nullptr, 100, false);
}

TEST(SieraTimer, SelfRestartFromCallback) {
    g_mgr_ptr       = &mgr;
    g_self_restarts = 0;
    siera_timer_start(&mgr, &g_self_timer, self_restart_cb, nullptr, 100, false);
    s_now = 100; siera_timer_tick(&mgr); CHECK_EQUAL(1, g_self_restarts);
    s_now = 200; siera_timer_tick(&mgr); CHECK_EQUAL(2, g_self_restarts);
}
