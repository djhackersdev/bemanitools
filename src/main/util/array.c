#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "iface-core/log.h"

#include "util/array.h"
#include "util/mem.h"

void array_init(struct array *array)
{
    memset(array, 0, sizeof(*array));
}

void array_remove_(size_t itemsz, struct array *array, size_t i)
{
    log_assert(i < array->nitems);

    array->nitems--;

    memmove(
        ((uint8_t *) array->items) + i * itemsz,
        ((uint8_t *) array->items) + (i + 1) * itemsz,
        (array->nitems - i) * itemsz);
}

void *array_reserve_(size_t itemsz, struct array *array, size_t nitems)
{
    size_t new_nalloced;
    size_t new_nitems;
    void *result;

    new_nitems = array->nitems + nitems;

    if (new_nitems > array->nalloced) {
        new_nalloced = array->nalloced;

        if (new_nalloced == 0) {
            new_nalloced = 1;
        }

        while (new_nalloced < new_nitems) {
            new_nalloced *= 2;
        }

        array->items = xrealloc(array->items, new_nalloced * itemsz);
        array->nalloced = new_nalloced;
    }

    result = ((uint8_t *) array->items) + array->nitems * itemsz;
    array->nitems = new_nitems;

    return result;
}

void array_fini(struct array *array)
{
    free(array->items);
}
