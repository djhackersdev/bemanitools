#ifndef ACIOTEST_KFCA_H
#define ACIOTEST_KFCA_H

#include <stdint.h>
#include <stdbool.h>

bool aciotest_kfca_handler_init(uint8_t node_id, void** ctx);
bool aciotest_kfca_handler_update(uint8_t node_id, void* ctx);

#endif
