#ifndef SIERA_COMMON_H
#define SIERA_COMMON_H

#include <stddef.h>
#include <stdint.h>

#define SIERA_OFFSET_OF(type, member) \
    ((size_t)&(((type *)0)->member))

#define SIERA_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - SIERA_OFFSET_OF(type, member)))

#define SIERA_NUM_ELEMENTS(array) \
    (sizeof(array) / sizeof((array)[0]))

#define SIERA_CONCAT_(a, b) a##b
#define SIERA_CONCAT(a, b)  SIERA_CONCAT_(a, b)

#endif
