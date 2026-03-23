#ifndef SIERA_SLIST_H
#define SIERA_SLIST_H

#include <stdbool.h>
#include "siera/common.h"

typedef struct siera_slist_node {
    struct siera_slist_node *next;
} siera_slist_node_t;

typedef struct {
    siera_slist_node_t *first;
} siera_slist_head_t;

#define SIERA_SLIST_HEAD_INIT { .first = NULL }

static inline void siera_slist_init(siera_slist_head_t *head)       { head->first = NULL; }
static inline void siera_slist_node_init(siera_slist_node_t *node)  { node->next = NULL; }
static inline bool siera_slist_is_empty(const siera_slist_head_t *h){ return h->first == NULL; }

static inline void siera_slist_push(siera_slist_head_t *head,
                                     siera_slist_node_t *node) {
    node->next  = head->first;
    head->first = node;
}

static inline siera_slist_node_t *siera_slist_pop(siera_slist_head_t *head) {
    siera_slist_node_t *node = head->first;
    if (node) { head->first = node->next; node->next = NULL; }
    return node;
}

static inline bool siera_slist_remove(siera_slist_head_t *head,
                                       siera_slist_node_t *node) {
    siera_slist_node_t **pp = &head->first;
    while (*pp) {
        if (*pp == node) { *pp = node->next; node->next = NULL; return true; }
        pp = &(*pp)->next;
    }
    return false;
}

#define SIERA_SLIST_FOR_EACH_SAFE(head, cursor, tmp)       \
    for ((cursor) = (head)->first,                         \
         (tmp) = (cursor) ? (cursor)->next : NULL;         \
         (cursor) != NULL;                                 \
         (cursor) = (tmp),                                 \
         (tmp) = (cursor) ? (cursor)->next : NULL)

#endif
