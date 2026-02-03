#include "list.h"

#include <stdlib.h>

void list_init(list_t* list)
{
  list->head = NULL;
}

void list_add_front(list_t* list, list_node_t* node)
{
  node->next = list->head;
  list->head = node;
}

void list_push(list_t* list, list_node_t* node)
{
  node->next = NULL;

  if(list->head == NULL) {
    list->head = node;
    return;
  }

  list_node_t* curr = list->head;

  while(curr->next != NULL) {
    curr = curr->next;
  }

  curr->next = node;
}

void list_delete(list_t* list, list_node_t* node)
{
  if(list->head == node) {
    list->head = list->head->next;
    return;
  }

  list_node_t* curr = list->head;

  while(curr->next != NULL) {
    if(curr->next == node) {
      curr->next = curr->next->next;
      return;
    }

    curr = curr->next;
  }
}

bool list_is_empty(list_t* list)
{
  return list->head == NULL;
}
