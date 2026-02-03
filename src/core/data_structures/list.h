#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct list_node_t {
  struct list_node_t* next;
} list_node_t;

typedef struct {
  list_node_t* head;
} list_t;

/**
 * @brief
 *
 * @param list
 */
void list_init(list_t* list);

/**
 * @brief
 *
 * @param list
 * @param node
 */
void list_add_front(list_t* list, list_node_t* node);

/**
 * @brief
 *
 * @param list
 * @param node
 */
void list_push(list_t* list, list_node_t* node);

/**
 * @brief
 *
 * @param list
 * @param node
 */
void list_delete(list_t* list, list_node_t* node);

#define list_for_each(list, iterator)                         \
  for(list_node_t* iterator = (list)->head; iterator != NULL; \
    iterator = iterator->next)

  /**
   * @brief 
   * 
   * @param list 
   * @return true 
   * @return false 
   */
  bool list_is_empty(list_t* list);