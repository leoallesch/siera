#include "siera/event.h"
#include "siera/common.h"

void siera_event_init(siera_event_t *event) {
    siera_slist_init(&event->subscribers);
}

void siera_event_sub_init(siera_event_sub_t *sub,
                           siera_event_cb_fn handler, void *ctx) {
    siera_slist_node_init(&sub->node);
    sub->handler = handler;
    sub->ctx     = ctx;
}

int siera_event_subscribe(siera_event_t *event, siera_event_sub_t *sub) {
    if (!event || !sub || !sub->handler) return -1;
    siera_slist_push(&event->subscribers, &sub->node);
    return 0;
}

int siera_event_unsubscribe(siera_event_t *event, siera_event_sub_t *sub) {
    if (!event || !sub) return -1;
    return siera_slist_remove(&event->subscribers, &sub->node) ? 0 : -1;
}

void siera_event_publish(siera_event_t *event, const void *args) {
    siera_slist_node_t *node, *tmp;
    SIERA_SLIST_FOR_EACH_SAFE(&event->subscribers, node, tmp) {
        siera_event_sub_t *sub = SIERA_CONTAINER_OF(node, siera_event_sub_t, node);
        sub->handler(args, sub->ctx);
    }
}
