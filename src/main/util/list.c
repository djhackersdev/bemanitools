#include <stddef.h>
#include <string.h>

#include "util/list.h"

extern inline struct list_node *list_peek_head(struct list *list);
extern inline const struct list_node *
list_peek_head_const(const struct list *list);

void list_init(struct list *list)
{
    memset(list, 0, sizeof(*list));
}

void list_append(struct list *list, struct list_node *node)
{
    node->next = NULL;

    if (list->tail != NULL) {
        list->tail->next = node;
    } else {
        list->head = node;
    }

    list->tail = node;
}

bool list_contains(struct list *list, struct list_node *node)
{
    struct list_node *pos;

    for (pos = list->head; pos != NULL; pos = pos->next) {
        if (pos == node) {
            return true;
        }
    }

    return false;
}

struct list_node *list_pop_head(struct list *list)
{
    struct list_node *node;

    if (list->head == NULL) {
        return NULL;
    }

    node = list->head;
    list->head = node->next;

    if (node->next == NULL) {
        list->tail = NULL;
    }

    node->next = NULL;

    return node;
}

void list_remove(struct list *list, struct list_node *node)
{
    struct list_node *pos;
    struct list_node *prev;

    for (prev = NULL, pos = list->head; pos != NULL;
         prev = pos, pos = pos->next) {
        if (pos == node) {
            if (prev != NULL) {
                prev->next = node->next;
            } else {
                list->head = node->next;
            }

            if (node->next == NULL) {
                list->tail = prev;
            }

            node->next = NULL;

            return;
        }
    }
}
