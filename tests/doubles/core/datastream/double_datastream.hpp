#ifndef DOUBLE_DATASTREAM_HPP
#define DOUBLE_DATASTREAM_HPP

#include "event_subscription.h"
#include "i_datastream.h"

#include <cstdbool>
#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct double_datastream double_datastream_t;

struct double_datastream {
  i_datastream_t interface;
};

void double_datastream_init(double_datastream_t* ds);

void double_datastream_reset(double_datastream_t* ds);

// Expectation helpers
void double_expect_contains(double_datastream_t* ds, datastream_key_t key, bool returns);
void double_expect_size(double_datastream_t* ds, datastream_key_t key, uint8_t returns);
void double_expect_read(double_datastream_t* ds, datastream_key_t key, const void* return_data, size_t size);
void double_expect_write(double_datastream_t* ds, datastream_key_t key, const void* expected_data, size_t size);
void double_expect_subscribe(double_datastream_t* ds, datastream_key_t key, event_subscription_t* sub);
void double_expect_subscribe_all(double_datastream_t* ds, event_subscription_t* sub);
void double_expect_unsubscribe(double_datastream_t* ds, event_subscription_t* sub);

void double_expect_no_calls(double_datastream_t* ds);

#ifdef __cplusplus
}
#endif

#endif
