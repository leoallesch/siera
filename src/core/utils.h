#pragma once

#define OFFSET_OF(type, member) ((size_t)&(((type*)0)->member))

#define CONTAINER_OF(ptr, type, member) \
  ({ ((type*)((char*)(ptr) - OFFSET_OF(type, member))); })

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#define NUM_ELEMENTS(array) (sizeof(array) / sizeof((array)[0]))
