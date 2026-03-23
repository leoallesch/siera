#ifndef SIERA_HAL_UART_H
#define SIERA_HAL_UART_H

#include <stdint.h>
#include <stddef.h>

typedef struct siera_hal_uart_t {
    int    (*write)(struct siera_hal_uart_t *self, const uint8_t *buf, size_t len);
    int    (*read)(struct siera_hal_uart_t *self, uint8_t *buf, size_t len, size_t *read_out);
    int    (*flush)(struct siera_hal_uart_t *self);
} siera_hal_uart_t;

static inline int siera_uart_write(siera_hal_uart_t *self, const uint8_t *buf, size_t len)
{
    return self->write(self, buf, len);
}

static inline int siera_uart_read(siera_hal_uart_t *self, uint8_t *buf, size_t len,
                                   size_t *read_out)
{
    return self->read(self, buf, len, read_out);
}

static inline int siera_uart_flush(siera_hal_uart_t *self)
{
    return self->flush(self);
}

#endif /* SIERA_HAL_UART_H */
