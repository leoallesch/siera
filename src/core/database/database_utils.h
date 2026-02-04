#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "utils.h"

#define DATABASE_EXPAND_AS_ENUM(name, type) name,

#define DATABASE_EXPAND_AS_STORAGE_STRUCT(name, type) uint8_t name[sizeof(type)];

#define DATABASE_ENUM(ENTRIES_LIST)       \
  enum {                                  \
    ENTRIES_LIST(DATABASE_EXPAND_AS_ENUM) \
  };

typedef struct s_database_storage_t s_database_storage_t;

#define DATABASE_STORAGE(ENTRIES_LIST)              \
  typedef struct s_database_storage_t {             \
    ENTRIES_LIST(DATABASE_EXPAND_AS_STORAGE_STRUCT) \
  } s_database_storage_t;

#define DATABASE_EXPAND_AS_ENTRY(name, type) { offsetof(s_database_storage_t, name), sizeof(type) },

//USAGE

// DATABASE_ENUM(DATABASE_ENTRIES)
// DATABASE_STORAGE(DATABASE_ENTRIES)

// static const s_database_entry_t database_entries[] = {
//   DATABASE_ENTRIES(DATABASE_EXPAND_AS_ENTRY)
// };

// static const s_database_config_t database_config = {
//   .entries = database_entries,
//   .count = NUM_ELEMENTS(database_entries),
// };
