#define LOG_MODULE "p4iodrv-device"

#include <windows.h>

#include "p4io/cmd.h"

#include "p4iodrv/device.h"
#include "p4iodrv/usb.h"

#include "util/log.h"
#include "util/mem.h"

struct p4iodrv_ctx {
    HANDLE jamma_handle;
    HANDLE bulk_handle;
    uint8_t seq_no;
};

static void p4io_cmd_init(struct p4iodrv_ctx *ctx);
static bool p4io_print_version(struct p4iodrv_ctx *ctx);

static bool p4io_transfer(
    struct p4iodrv_ctx *ctx,
    uint8_t cmd,
    const void *req_payload,
    size_t req_payload_len,
    void *resp_payload,
    size_t resp_payload_len
) {
    bool ret = p4io_usb_transfer(ctx->bulk_handle, cmd, ctx->seq_no,
        req_payload, req_payload_len,
        resp_payload, &resp_payload_len);

    ctx->seq_no++;

    return ret;
}

struct p4iodrv_ctx *p4iodrv_open(void) {
    struct p4iodrv_ctx *ctx = xcalloc(sizeof(struct p4iodrv_ctx));

    // jamma is read as fast as possible in its own thread, so having them be
    // separate files is absolutely essential to avoid the bulk handle being
    // starved
    ctx->jamma_handle = p4io_usb_open();
    ctx->bulk_handle = p4io_usb_open();
    ctx->seq_no = 0;

    if(ctx->jamma_handle == INVALID_HANDLE_VALUE || ctx->bulk_handle == INVALID_HANDLE_VALUE) {
        free(ctx);
        return NULL;
    }

    p4io_cmd_init(ctx);

    if(!p4io_print_version(ctx)) {
        free(ctx);
        return NULL;
    }

    return ctx;
}

void p4iodrv_close(struct p4iodrv_ctx *ctx) {
    p4io_usb_close(ctx->jamma_handle);
    p4io_usb_close(ctx->bulk_handle);
    free(ctx);
}

bool p4iodrv_read_jamma(struct p4iodrv_ctx *ctx, uint32_t jamma[4]) {
    return p4io_usb_read_jamma(ctx->jamma_handle, jamma);
}

// send something you don't expect a response for
static bool p4io_send(struct p4iodrv_ctx *ctx, uint8_t cmd) {
    uint8_t dummy[P4IO_MAX_PAYLOAD];
    return p4io_transfer(ctx, cmd, NULL, 0, dummy, sizeof(dummy));
}

// real IO does not check the return value, so neither do we
static void p4io_cmd_init(struct p4iodrv_ctx *ctx) {
    p4io_send(ctx, P4IO_CMD_INIT);
}

static bool p4io_print_version(struct p4iodrv_ctx *ctx) {
    char p4io_name[128];

    if(!p4io_usb_read_device_name(ctx->bulk_handle, p4io_name)) {
        return false;
    }

    struct p4io_resp_device_info dev_info;
    if(!p4iodrv_cmd_device_info(ctx, &dev_info)) {
        log_warning("p4io get_device_info failed");
        return false;
    }

    dev_info.build_date[15] = '\0';
    dev_info.build_time[15] = '\0';

    log_info("p4io name: %s", p4io_name);
    log_info("p4io type: %08X", dev_info.type);
    log_info("p4io version: %d.%d.%d", dev_info.version_major, dev_info.version_minor, dev_info.version_revision);
    log_info("p4io product: %.4s", dev_info.product_code);
    log_info("p4io build date: %.16s", dev_info.build_date);
    log_info("p4io build time: %.16s", dev_info.build_time);
    return true;
}

bool p4iodrv_cmd_device_info(struct p4iodrv_ctx *ctx, struct p4io_resp_device_info *info) {
    return p4io_transfer(ctx, P4IO_CMD_GET_DEVICE_INFO, NULL, 0, info, sizeof(*info));
}

bool p4iodrv_cmd_portout(struct p4iodrv_ctx *ctx, const uint8_t buffer[16]) {
    uint8_t dummy[P4IO_MAX_PAYLOAD];
    return p4io_transfer(ctx, P4IO_CMD_SET_PORTOUT, buffer, 16, dummy, sizeof(dummy));
}

bool p4iodrv_cmd_coinstock(struct p4iodrv_ctx *ctx, const uint8_t buffer[4]) {
    uint8_t dummy[P4IO_MAX_PAYLOAD];
    return p4io_transfer(ctx, P4IO_CMD_COINSTOCK, buffer, 4, dummy, sizeof(dummy));
}
