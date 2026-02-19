#pragma once

#include <stdint.h>

#include "utils.h"

typedef uint8_t menu_index_t;

typedef struct {
  uint8_t list_count;
  menu_index_t current_index;
} menu_iter_t;

static inline menu_iter_t menu_iter(uint8_t list_count)
{
  return (menu_iter_t){
    .list_count = list_count,
    .current_index = 0
  };
}

static inline uint8_t menu_list_move(menu_iter_t menu_list, int8_t delta)
{
  return WRAP(menu_list.current_index, delta, menu_list.list_count);
}
