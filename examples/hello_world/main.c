#include <stdint.h>
#include <stdio.h>

#include "database.h"
#include "database_utils.h"
#include "event_subscription.h"
#include "timer.h"
#include "timesource_simulator.h"
#include "utils.h"

#define DATABASE(ENTRY) \
  ENTRY(Key_TestInt, uint16_t)

DATABASE_ENUM(DATABASE)
DATABASE_STORAGE(DATABASE)

static const s_database_entry_t database_entries[] = {
  DATABASE(DATABASE_EXPAND_AS_ENTRY)
};

static const s_database_config_t config = {
  database_entries, NUM_ELEMENTS(database_entries)
};

s_timer_controller_t timer_controller;
composite_datastream_t database;
ram_storage_t storage;
s_timer_t timer;
event_subscription_t sub;

void timer_cb(void* context)
{
  composite_datastream_t* db = (composite_datastream_t*)context;
  uint16_t val;
  database_read(db, Key_TestInt, &val);
  val++;
  database_write(db, Key_TestInt, &val);
}

void on_change(void* context, const void* data)
{
  (void)context;
  const s_database_on_change_args_t* args = (const s_database_on_change_args_t*)data;
  printf("Database key %d changed to %d.\n", args->key, *(uint16_t*)args->data);
}

int main(void)
{
  printf("Starting Hello World Example\n");
  timer_controller_init(&timer_controller, timesource_simulator());
  database_init(&database, &config, &storage);

  event_subscription_init(&sub, on_change, NULL);
  timer_start_repeating(&timer, &timer_controller, 1000, timer_cb, &database);

  database_subscribe_all(&database, &sub);

  while(timer_controller_run(&timer_controller)) {
    // Main loop
  }
}
