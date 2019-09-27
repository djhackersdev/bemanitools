#ifndef UTIL_ARRAY_H
#define UTIL_ARRAY_H

#include <stddef.h>

struct array {
    void *items;
    size_t nitems;
    size_t nalloced;
};

#define array_item(type, array, i) \
        (&(((type *) (array)->items)[i]))

#define array_reserve(type, array, nitems) \
        ((type *) array_reserve_(sizeof(type), array, nitems))

#define array_remove(type, array, i) \
        array_remove_(sizeof(type), array, i)

#define array_append(type, array) \
        array_reserve(type, array, 1)

void array_init(struct array *array);
void *array_reserve_(size_t itemsz, struct array *array, size_t nitems);
void array_remove_(size_t itemsz, struct array *array, size_t i);
void array_fini(struct array *array);

#endif
