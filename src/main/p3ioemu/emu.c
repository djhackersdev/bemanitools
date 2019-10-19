#define LOG_MODULE "p3ioemu"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "hook/iohook.h"

#include "p3io/cmd.h"
#include "p3io/frame.h"
#include "p3io/ioctl.h"

#include "p3ioemu/devmgr.h"
#include "p3ioemu/emu.h"
#include "p3ioemu/uart.h"

#include "util/iobuf.h"
#include "util/log.h"

static HANDLE p3io_emu_fd;
static uint8_t p3io_emu_resp_bytes[256];
static struct iobuf p3io_emu_resp;
static const struct p3io_ops *p3io_ops;
static void *p3io_ops_ctx;

static HRESULT p3io_emu_handle_open(struct irp *irp);
static HRESULT p3io_emu_handle_close(struct irp *irp);
static HRESULT p3io_emu_handle_ioctl(struct irp *irp);
static HRESULT p3io_emu_handle_read(struct irp *irp);
static HRESULT p3io_emu_handle_write(struct irp *irp);

static HRESULT p3io_cmd_dispatch(const union p3io_req_any *req);

static void p3io_cmd_get_version(
    const struct p3io_hdr *req, struct p3io_resp_version *resp);

static void
p3io_cmd_set_watchdog(const struct p3io_req_u8 *req, struct p3io_resp_u8 *resp);

static void p3io_cmd_set_outputs(
    const struct p3io_req_set_outputs *req, struct p3io_resp_u8 *resp);

static void p3io_cmd_read_plug(
    const struct p3io_req_read_plug *req, struct p3io_resp_read_plug *resp);

static void p3io_cmd_get_cab_type_or_dipsw(
    const struct p3io_req_get_cab_type_or_dipsw *req,
    struct p3io_resp_get_cab_type_or_dipsw *resp);

static void p3io_cmd_get_video_freq(
    const struct p3io_req_u8 *req, struct p3io_resp_u8 *resp);

static void
p3io_cmd_set_mode(const struct p3io_req_u8 *req, struct p3io_resp_u8 *resp);

static void p3io_cmd_get_coinstock(
    const struct p3io_req_u8 *req, struct p3io_resp_coin_stock *resp);

static void p3io_cmd_set_coin_counter(
    const struct p3io_req_set_coin_counter *req, struct p3io_resp_u8 *resp);

static void
p3io_cmd_unknown(const union p3io_req_any *req, struct p3io_resp_u8 *resp);

void p3io_emu_init(const struct p3io_ops *ops, void *ctx)
{
    log_assert(p3io_emu_fd == NULL);
    log_assert(ops != NULL);

    p3io_emu_resp.bytes = p3io_emu_resp_bytes;
    p3io_emu_resp.nbytes = sizeof(p3io_emu_resp_bytes);
    p3io_emu_resp.pos = 0;

    p3io_ops = ops;
    p3io_ops_ctx = ctx;

    p3io_emu_fd = iohook_open_dummy_fd();
}

void p3io_emu_fini(void)
{
    if (p3io_emu_fd == NULL) {
        return;
    }

    CloseHandle(p3io_emu_fd);
    p3io_emu_fd = NULL;

    if (p3io_ops->close != NULL) {
        p3io_ops->close(p3io_ops_ctx);
    }
}

HRESULT
p3io_emu_dispatch_irp(struct irp *irp)
{
    log_assert(irp != NULL);

    if (irp->op != IRP_OP_OPEN && irp->fd != p3io_emu_fd) {
        return irp_invoke_next(irp);
    }

    switch (irp->op) {
        case IRP_OP_OPEN:
            return p3io_emu_handle_open(irp);
        case IRP_OP_CLOSE:
            return p3io_emu_handle_close(irp);
        case IRP_OP_READ:
            return p3io_emu_handle_read(irp);
        case IRP_OP_WRITE:
            return p3io_emu_handle_write(irp);
        case IRP_OP_IOCTL:
            return p3io_emu_handle_ioctl(irp);
        default:
            return E_NOTIMPL;
    }
}

static HRESULT p3io_emu_handle_open(struct irp *irp)
{
    if (!p3io_setupapi_match_path(irp->open_filename)) {
        return irp_invoke_next(irp);
    }

    log_info("P3IO device opened");
    irp->fd = p3io_emu_fd;

    return S_OK;
}

static HRESULT p3io_emu_handle_close(struct irp *irp)
{
    log_info("P3IO device closed");

    return S_OK;
}

static HRESULT p3io_emu_handle_ioctl(struct irp *irp)
{
    uint32_t *dest;
    uint32_t pad;
    HRESULT hr;

    if (irp->ioctl != P3IO_IOCTL_READ_JAMMA) {
        log_warning("Unknown ioctl: %x", irp->ioctl);

        return E_NOTIMPL;
    }

    if (irp->read.nbytes < sizeof(uint32_t)) {
        log_warning("Insufficient ioctl response buffer space");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    pad = 0;

    if (p3io_ops->read_jamma != NULL) {
        hr = p3io_ops->read_jamma(p3io_ops_ctx, &pad);

        if (FAILED(hr)) {
            return hr;
        }
    }

    dest = (uint32_t *) irp->read.bytes;
    *dest = ~_byteswap_ulong(pad);
    irp->read.pos = sizeof(pad);

    return S_OK;
}

static HRESULT p3io_emu_handle_read(struct irp *irp)
{
    struct const_iobuf tmp;

    iobuf_flip(&tmp, &p3io_emu_resp);
    p3io_emu_resp.pos = 0;

    return iobuf_move(&irp->read, &tmp);
}

static HRESULT p3io_emu_handle_write(struct irp *irp)
{
    union p3io_req_any req;
    struct iobuf deframe;
    HRESULT hr;

    memset(&req, 0, sizeof(req));

    deframe.bytes = req.raw;
    deframe.nbytes = sizeof(req.raw);
    deframe.pos = 0;

    hr = p3io_frame_decode(&deframe, &irp->write);

    if (FAILED(hr)) {
        return hr;
    }

    if (deframe.pos < sizeof(req.hdr)) {
        log_warning("Received truncated P3IO frame");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    p3io_emu_resp.pos = 0;

    return p3io_cmd_dispatch(&req);
}

static HRESULT p3io_cmd_dispatch(const union p3io_req_any *req)
{
    union p3io_resp_any resp;
    uint8_t cmd;

    cmd = p3io_req_cmd(req);

    switch (cmd) {
        case P3IO_CMD_GET_VERSION:
            p3io_cmd_get_version(&req->hdr, &resp.version);

            break;

        case P3IO_CMD_SET_WATCHDOG:
            p3io_cmd_set_watchdog(&req->u8, &resp.u8);

            break;

        case P3IO_CMD_SET_OUTPUTS:
            p3io_cmd_set_outputs(&req->set_outputs, &resp.u8);

            break;

        case P3IO_CMD_READ_PLUG:
            p3io_cmd_read_plug(&req->read_plug, &resp.read_plug);

            break;

        case P3IO_CMD_GET_CAB_TYPE_OR_DIPSW:
            p3io_cmd_get_cab_type_or_dipsw(
                &req->cab_type_or_dipsw, &resp.cab_type_or_dipsw);

            break;

        case P3IO_CMD_GET_VIDEO_FREQ:
            p3io_cmd_get_video_freq(&req->u8, &resp.u8);

            break;

        case P3IO_CMD_SET_MODE:
            p3io_cmd_set_mode(&req->u8, &resp.u8);

            break;

        case P3IO_CMD_GET_COINSTOCK:
            p3io_cmd_get_coinstock(&req->u8, &resp.coin_stock);

            break;

        case P3IO_CMD_SET_COINCOUNTER:
            p3io_cmd_set_coin_counter(&req->set_coin_counter, &resp.u8);

            break;

        case P3IO_CMD_RS232_OPEN_CLOSE:
            p3io_uart_cmd_open_close(&req->rs232_open_close, &resp.u8);

            break;

        case P3IO_CMD_RS232_READ:
            p3io_uart_cmd_read(&req->rs232_read, &resp.rs232_read);

            break;

        case P3IO_CMD_RS232_WRITE:
            p3io_uart_cmd_write(&req->rs232_write, &resp.rs232_write);

            break;

        default:
            p3io_cmd_unknown(req, &resp.u8);

            break;
    }

    p3io_frame_encode(&p3io_emu_resp, &resp, resp.hdr.nbytes + 1);

    return S_OK;
}

static void
p3io_cmd_get_version(const struct p3io_hdr *req, struct p3io_resp_version *resp)
{
    log_misc("%s", __func__);

    p3io_resp_init(&resp->hdr, sizeof(resp), req);
    resp->status = 0;
    resp->str[0] = 'H';
    resp->str[1] = 'D';
    resp->str[2] = 'X';
    resp->str[3] = '\0';
    resp->major = 1;
    resp->minor = 2;
    resp->patch = 3;
}

static void
p3io_cmd_set_watchdog(const struct p3io_req_u8 *req, struct p3io_resp_u8 *resp)
{
    log_misc("%s(%02x)", __func__, req->u8);

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);
    resp->status = 0;
    resp->u8 = 0;
}

static void p3io_cmd_set_outputs(
    const struct p3io_req_set_outputs *req, struct p3io_resp_u8 *resp)
{
    uint32_t outputs;
    HRESULT hr;

    if (p3io_ops->set_outputs != NULL) {
        outputs = _byteswap_ulong(req->outputs);
        hr = p3io_ops->set_outputs(p3io_ops_ctx, outputs);
    } else {
        hr = S_OK;
    }

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);
    resp->status = FAILED(hr);
    resp->u8 = 0;
}

static void p3io_cmd_read_plug(
    const struct p3io_req_read_plug *req, struct p3io_resp_read_plug *resp)
{
    HRESULT hr;

    log_misc("%s(%02x)", __func__, req->flags);

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);
    resp->present = 1;

    /*
     * Some notes about the flags available here:
     * 0x10: black plug rom read
     * 0x12: black plug eeprom read
     * 0x00: white plug rom read
     * 0x02: white plug eeprom read
     *
     * The structure of the buffer is the same for all flags set and the game
     * just grabs whatever it currently needs from the buffer. i.e. if we
     * send everything on every read (rom + eeprom) always, the game doesn't
     * care
     */

    if (req->flags & 0x10) {
        if (p3io_ops->get_roundplug) {
            hr = p3io_ops->get_roundplug(
                p3io_ops_ctx, 0, resp->rom, resp->eeprom);
        } else {
            log_warning("Reading black roundplug but no function available");
            memset(resp->rom, 0, 8);
            memset(resp->eeprom, 0, 32);
            hr = S_OK;
        }
    } else {
        if (p3io_ops->get_roundplug) {
            hr = p3io_ops->get_roundplug(
                p3io_ops_ctx, 1, resp->rom, resp->eeprom);
        } else {
            log_warning("Reading white roundplug but no function available");
            memset(resp->rom, 0, 8);
            memset(resp->eeprom, 0, 32);
            hr = S_OK;
        }
    }

    log_misc(
        "Reading %s roundplug, success 0x%lX, rom %02X%02X%02X%02X%02X%02X"
        "%02X%02X, eeprom sig %X%X%X%X%X%X",
        req->flags & 0x10 ? "black" : "white",
        hr,
        resp->rom[0],
        resp->rom[1],
        resp->rom[2],
        resp->rom[3],
        resp->rom[4],
        resp->rom[5],
        resp->rom[6],
        resp->rom[7],
        resp->eeprom[0],
        resp->eeprom[1],
        resp->eeprom[2],
        resp->eeprom[3],
        resp->eeprom[4],
        resp->eeprom[5]);

    resp->status = FAILED(hr);
}

static void p3io_cmd_get_cab_type_or_dipsw(
    const struct p3io_req_get_cab_type_or_dipsw *req,
    struct p3io_resp_get_cab_type_or_dipsw *resp)
{
    HRESULT hr;
    uint8_t dipsw;
    enum p3io_cab_type type;

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);

    hr = S_OK;

    if (req->cab_type_or_dipsw == 0) {
        if (p3io_ops->get_cab_type) {
            hr = p3io_ops->get_cab_type(p3io_ops_ctx, &type);

            if (hr == S_OK) {
                switch (type) {
                    case P3IO_CAB_TYPE_SD:
                        log_misc("%s: Returning cab type SD", __func__);
                        resp->status = 1;
                        break;

                    case P3IO_CAB_TYPE_HD:
                        log_misc("%s: Returning cab type HD", __func__);
                        resp->status = 2;
                        break;

                    default:
                        log_assert(false);
                        break;
                }
            } else {
                resp->status = 0;
            }
        } else {
            resp->status = 0;
        }
    } else if (req->cab_type_or_dipsw == 1) {
        if (p3io_ops->get_dipsw) {
            hr = p3io_ops->get_dipsw(p3io_ops_ctx, &dipsw);

            if (hr == S_OK) {
                resp->status = dipsw;
            } else {
                resp->status = 0;
            }
        } else {
            resp->status = 0;
        }
    } else {
        log_warning(
            "Unknown value for cab type or dipsw: %d", req->cab_type_or_dipsw);
        resp->status = 0;
    }
}

static void p3io_cmd_get_video_freq(
    const struct p3io_req_u8 *req, struct p3io_resp_u8 *resp)
{
    HRESULT hr;
    enum p3io_video_freq freq;

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);

    if (p3io_ops->get_video_freq != NULL) {
        hr = p3io_ops->get_video_freq(p3io_ops_ctx, &freq);
    } else {
        hr = S_OK;
        freq = P3IO_VIDEO_FREQ_31KHZ;
    }

    if (hr == S_OK) {
        switch (freq) {
            case P3IO_VIDEO_FREQ_15KHZ:
                log_misc("%s: Returning 15 kHz", __func__);
                resp->status = 0;
                resp->u8 = 0x00;

                break;

            case P3IO_VIDEO_FREQ_31KHZ:
                log_misc("%s: Returning 31 kHz", __func__);
                resp->status = 0;
                resp->u8 = 0x80;

                break;

            default:
                log_assert(false);
                break;
        }
    } else {
        log_misc("%s: Returning error! (hr=%x)", __func__, (int) hr);
        resp->status = 1;
        resp->u8 = 0x00;
    }
}

static void
p3io_cmd_set_mode(const struct p3io_req_u8 *req, struct p3io_resp_u8 *resp)
{
    log_misc("%s(%02x)", __func__, req->u8);

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);
    resp->status = 0;
    resp->u8 = 0;
}

static void p3io_cmd_get_coinstock(
    const struct p3io_req_u8 *req, struct p3io_resp_coin_stock *resp)
{
    uint16_t slots[2];
    HRESULT hr;

    memset(slots, 0, sizeof(slots));

    if (p3io_ops->get_coinstock != NULL) {
        hr = p3io_ops->get_coinstock(p3io_ops_ctx, slots, lengthof(slots));
    } else {
        hr = S_OK;
    }

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);
    resp->status = 0;
    resp->error = FAILED(hr);
    resp->slots[0] = _byteswap_ushort(slots[0]);
    resp->slots[1] = _byteswap_ushort(slots[1]);
}

static void p3io_cmd_set_coin_counter(
    const struct p3io_req_set_coin_counter *req, struct p3io_resp_u8 *resp)
{
    log_misc(
        "%s(%02x %02x)", __func__, req->coin_counter[0], req->coin_counter[1]);

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);
    resp->status = 0;
    resp->u8 = 0;
}

static void
p3io_cmd_unknown(const union p3io_req_any *req, struct p3io_resp_u8 *resp)
{
    log_warning("Unsupported P3IO command: %02x", p3io_req_cmd(req));

    p3io_resp_init(&resp->hdr, sizeof(*resp), &req->hdr);
    resp->status = 1;
    resp->u8 = 0;
}
