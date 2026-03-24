#ifndef SIERA_VIEW_H
#define SIERA_VIEW_H

typedef struct siera_view_t {
  void (*load)(struct siera_view_t *self, void *canvas);
  void (*unload)(struct siera_view_t *self);
} siera_view_t;

static inline void siera_view_load(siera_view_t *self, void *canvas)
{
  self->load(self, canvas);
}

static inline void siera_view_unload(siera_view_t *self)
{
  self->unload(self);
}

#endif /* SIERA_VIEW_H */
