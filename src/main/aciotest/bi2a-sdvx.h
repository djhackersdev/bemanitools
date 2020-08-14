#ifndef ACIOTEST_BI2A_SDVX_H
#define ACIOTEST_BI2A_SDVX_H

#include <stdbool.h>
#include <stdint.h>

bool aciotest_bi2a_sdvx_handler_init(uint8_t node_id, void **ctx);
bool aciotest_bi2a_sdvx_handler_update(uint8_t node_id, void *ctx);

#endif
