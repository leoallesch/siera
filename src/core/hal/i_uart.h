#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "event.h"

typedef struct i_uart_t {
  /**
   * @brief Write a single byte to the UART transmit buffer.
   *
   * @param self  Pointer to the UART instance.
   * @param byte  Byte value to transmit.
   */
  void (*write_byte)(struct i_uart_t* self, uint8_t byte);

  /**
   * @brief Read a single byte from the UART receive buffer.
   *
   * @param self  Pointer to the UART instance.
   * @param out   Pointer to store the received byte.
   * @return true if a byte was available and written to out; false if the buffer was empty.
   */
  bool (*read_byte)(struct i_uart_t* self, uint8_t* out);

  /**
   * @brief Write a buffer of bytes to the UART transmit buffer.
   *
   * @param self  Pointer to the UART instance.
   * @param buf   Pointer to the data buffer to transmit.
   * @param len   Number of bytes to transmit.
   */
  void (*write_bytes)(struct i_uart_t* self, const uint8_t* buf, uint16_t len);

  /**
   * @brief Get the event handler for UART receive events.
   *
   * @param self  Pointer to the UART instance.
   * @return Pointer to the receive event handler.
   */
  event_t *(*on_receieve)(struct i_uart_t* self);

  /**
   * @brief Get the event handler for UART send complete events.
   *
   * @param self  Pointer to the UART instance.
   * @return Pointer to the send complete event handler.
   */
  event_t *(*on_send_complete)(struct i_uart_t* self);
} i_uart_t;

static inline void uart_write_byte(i_uart_t* self, uint8_t byte)
{
  self->write_byte(self, byte);
}

static inline bool uart_read_byte(i_uart_t* self, uint8_t* out)
{
  return self->read_byte(self, out);
}

static inline void uart_write_bytes(i_uart_t* self, const uint8_t* buf, uint16_t len)
{
  self->write_bytes(self, buf, len);
}

static inline event_t* uart_on_receive(i_uart_t* self)
{
  return self->on_receieve(self);
}

static inline event_t* uart_on_send_complete(i_uart_t* self)
{
  return self->on_send_complete(self);
}
