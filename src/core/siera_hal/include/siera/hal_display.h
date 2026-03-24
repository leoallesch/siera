#ifndef SIERA_HAL_DISPLAY_H
#define SIERA_HAL_DISPLAY_H

typedef struct siera_hal_display_t {
  void *(*get_canvas)(struct siera_hal_display_t *self);
  void  (*flush)(struct siera_hal_display_t *self);
} siera_hal_display_t;

static inline void *siera_hal_display_get_canvas(siera_hal_display_t *self)
{
  return self->get_canvas(self);
}

static inline void siera_hal_display_flush(siera_hal_display_t *self)
{
  self->flush(self);
}

#endif /* SIERA_HAL_DISPLAY_H */
