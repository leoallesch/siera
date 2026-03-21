#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct i_flash_kv_t {
  /**
   * @brief Write a blob value under the given key.
   *
   * @param self  Pointer to the flash KV instance.
   * @param key   Null-terminated key name string.
   * @param data  Pointer to data to write.
   * @param size  Number of bytes to write.
   */
  void (*write)(struct i_flash_kv_t* self, const char* key, const void* data, uint8_t size);

  /**
   * @brief Read a blob value for the given key.
   *
   * @param self  Pointer to the flash KV instance.
   * @param key   Null-terminated key name string.
   * @param out   Output buffer to read into.
   * @param size  Number of bytes to read.
   * @return true if the key existed and data was read; false if not found.
   */
  bool (*read)(struct i_flash_kv_t* self, const char* key, void* out, uint8_t size);

  /**
   * @brief Erase a key-value pair from flash.
   *
   * @param self  Pointer to the flash KV instance.
   * @param key   Null-terminated key name string.
   */
  void (*erase)(struct i_flash_kv_t* self, const char* key);
} i_flash_kv_t;

static inline void flash_kv_write(i_flash_kv_t* self, const char* key, const void* data, uint8_t size)
{
  self->write(self, key, data, size);
}

static inline bool flash_kv_read(i_flash_kv_t* self, const char* key, void* out, uint8_t size)
{
  return self->read(self, key, out, size);
}

static inline void flash_kv_erase(i_flash_kv_t* self, const char* key)
{
  self->erase(self, key);
}
