#include "esp32_flash_kv.h"

static void write(i_flash_kv_t* self, const char* key, const void* data, uint8_t size)
{
  esp32_flash_kv_t* instance = (esp32_flash_kv_t*)self;
  nvs_set_blob(instance->handle, key, data, size);
  nvs_commit(instance->handle);
}

static bool read(i_flash_kv_t* self, const char* key, void* out, uint8_t size)
{
  esp32_flash_kv_t* instance = (esp32_flash_kv_t*)self;
  size_t blob_len = size;
  esp_err_t err = nvs_get_blob(instance->handle, key, out, &blob_len);
  return err == ESP_OK;
}

static void erase(i_flash_kv_t* self, const char* key)
{
  esp32_flash_kv_t* instance = (esp32_flash_kv_t*)self;
  nvs_erase_key(instance->handle, key);
  nvs_commit(instance->handle);
}

void esp32_flash_kv_init(esp32_flash_kv_t* instance, const char* namespace)
{
  nvs_flash_init();
  nvs_open(namespace, NVS_READWRITE, &instance->handle);

  instance->interface = (i_flash_kv_t){
    .write = write,
    .read  = read,
    .erase = erase,
  };
}
