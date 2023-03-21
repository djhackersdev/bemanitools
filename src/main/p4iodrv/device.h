#ifndef P4IODRV_DEVICE_H
#define P4IODRV_DEVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "p4io/cmd.h"

struct p4iodrv_ctx;

struct p4iodrv_ctx *p4iodrv_open(void);
void p4iodrv_close(struct p4iodrv_ctx *ctx);
bool p4iodrv_cmd_device_info(
    struct p4iodrv_ctx *ctx, struct p4io_resp_device_info *info);
bool p4iodrv_cmd_portout(struct p4iodrv_ctx *ctx, const uint8_t buffer[16]);
bool p4iodrv_cmd_coinstock(struct p4iodrv_ctx *ctx, const uint8_t buffer[4]);
bool p4iodrv_read_jamma(struct p4iodrv_ctx *ctx, uint32_t jamma[4]);

#endif
