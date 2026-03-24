#include "siera/esp32_uart.h"

#include "driver/uart.h"

typedef struct {
  siera_hal_uart_t interface;
  int              port;
} siera_esp32_uart_t;

static siera_esp32_uart_t s_uart;

static int _write(siera_hal_uart_t* self, const uint8_t* buf, size_t len)
{
  siera_esp32_uart_t* drv = (siera_esp32_uart_t*)self;
  int sent = uart_write_bytes((uart_port_t)drv->port, (const char*)buf, len);
  return (sent < 0) ? -1 : 0;
}

static int _read(siera_hal_uart_t* self, uint8_t* buf, size_t len, size_t* read_out)
{
  siera_esp32_uart_t* drv = (siera_esp32_uart_t*)self;
  int n = uart_read_bytes((uart_port_t)drv->port, buf, len, 0);
  if(n < 0)
    return -1;
  *read_out = (size_t)n;
  return 0;
}

static int _flush(siera_hal_uart_t* self)
{
  siera_esp32_uart_t* drv = (siera_esp32_uart_t*)self;
  return uart_flush((uart_port_t)drv->port) == ESP_OK ? 0 : -1;
}

siera_hal_uart_t* siera_esp32_uart_init(int port)
{
  s_uart.port           = port;
  s_uart.interface.write = _write;
  s_uart.interface.read  = _read;
  s_uart.interface.flush = _flush;
  return &s_uart.interface;
}
