#ifndef UTIL_LIST_H
#define UTIL_LIST_H

#include <stdbool.h>
#include <stddef.h>

struct list {
    struct list_node *head;
    struct list_node *tail;
};

struct list_node {
    struct list_node *next;
};

void list_init(struct list *list);
void list_append(struct list *list, struct list_node *node);
bool list_contains(struct list *list, struct list_node *node);
inline struct list_node *list_peek_head(struct list *list);
inline const struct list_node *list_peek_head_const(const struct list *list);
struct list_node *list_pop_head(struct list *list);
void list_remove(struct list *list, struct list_node *node);

inline struct list_node *list_peek_head(struct list *list)
{
    return list->head;
}

inline const struct list_node *list_peek_head_const(const struct list *list)
{
    return list->head;
}

#endif
