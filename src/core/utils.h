#pragma once

#define OFFSET_OF(type, member) ((size_t)&(((type*)0)->member))

#define CONTAINER_OF(ptr, type, member) \
  ({ ((type*)((char*)(ptr) - OFFSET_OF(type, member))); })
