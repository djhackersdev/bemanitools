#pragma once

#include "aio.h"

void init_aio_handles(void);

struct bi2x_ctx;

struct bi2x_ctx *setup_bi2x(void);

void close_bi2x(struct bi2x_ctx *ctx);

void poll_bi2x(struct bi2x_ctx *ctx, struct AIO_IOB2_BI2X_TBS__DEVSTATUS *status);
