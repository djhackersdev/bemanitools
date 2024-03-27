#ifndef API_IO_BST_H
#define API_IO_BST_H

#include <stdbool.h>
#include <stdint.h>

typedef struct api_io_bst api_io_bst_t;

void api_io_bst_load(const char *path, api_io_bst_t **io);
void api_io_bst_free(api_io_bst_t **io);

bool api_io_bst_init(const api_io_bst_t *io);
void api_io_bst_fini(const api_io_bst_t *io);
bool api_io_bst_input_read(const api_io_bst_t *io);
uint8_t api_io_bst_input_get(const api_io_bst_t *io);

#endif