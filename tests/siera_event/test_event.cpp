extern "C" {
#include "siera/event.h"
}
#include <CppUTest/TestHarness.h>

static int s_count;
static void handler(const void *, void *) { s_count++; }

TEST_GROUP(SieraEvent) {};

TEST(SieraEvent, Publish) {
    siera_event_t event;
    siera_event_init(&event);
    siera_event_sub_t sub;
    siera_event_sub_init(&sub, handler, nullptr);
    siera_event_subscribe(&event, &sub);
    s_count = 0;
    siera_event_publish(&event, nullptr);
    CHECK_EQUAL(1, s_count);
}

TEST(SieraEvent, Unsubscribe) {
    siera_event_t event;
    siera_event_init(&event);
    siera_event_sub_t sub;
    siera_event_sub_init(&sub, handler, nullptr);
    siera_event_subscribe(&event, &sub);
    siera_event_unsubscribe(&event, &sub);
    s_count = 0;
    siera_event_publish(&event, nullptr);
    CHECK_EQUAL(0, s_count);
}

static siera_event_sub_t g_self;
static void self_remover(const void *, void *ctx) {
    s_count++;
    siera_event_unsubscribe(static_cast<siera_event_t *>(ctx), &g_self);
}

TEST(SieraEvent, SelfRemoval) {
    siera_event_t event;
    siera_event_init(&event);
    siera_event_sub_t other;
    siera_event_sub_init(&other, handler, nullptr);
    siera_event_subscribe(&event, &other);
    siera_event_sub_init(&g_self, self_remover, &event);
    siera_event_subscribe(&event, &g_self);
    s_count = 0;
    siera_event_publish(&event, nullptr);
    CHECK_EQUAL(2, s_count);
    s_count = 0;
    siera_event_publish(&event, nullptr);
    CHECK_EQUAL(1, s_count);
}

TEST(SieraEvent, Empty) {
    siera_event_t event;
    siera_event_init(&event);
    // Should not crash
    siera_event_publish(&event, nullptr);
}

TEST(SieraEvent, MultipleSubscribers) {
    siera_event_t event;
    siera_event_init(&event);
    siera_event_sub_t s1, s2, s3;
    siera_event_sub_init(&s1, handler, nullptr);
    siera_event_sub_init(&s2, handler, nullptr);
    siera_event_sub_init(&s3, handler, nullptr);
    siera_event_subscribe(&event, &s1);
    siera_event_subscribe(&event, &s2);
    siera_event_subscribe(&event, &s3);
    s_count = 0;
    siera_event_publish(&event, nullptr);
    CHECK_EQUAL(3, s_count);
}
