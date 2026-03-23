#ifndef SIERA_EVENT_H
#define SIERA_EVENT_H

#include "siera/slist.h"

typedef void (*siera_event_cb_fn)(const void *args, void *ctx);

typedef struct {
    siera_slist_node_t node;
    siera_event_cb_fn  handler;
    void              *ctx;
} siera_event_sub_t;

typedef struct {
    siera_slist_head_t subscribers;
} siera_event_t;

void siera_event_init(siera_event_t *event);
void siera_event_sub_init(siera_event_sub_t *sub, siera_event_cb_fn handler, void *ctx);
int  siera_event_subscribe(siera_event_t *event, siera_event_sub_t *sub);
int  siera_event_unsubscribe(siera_event_t *event, siera_event_sub_t *sub);
void siera_event_publish(siera_event_t *event, const void *args);

#endif
