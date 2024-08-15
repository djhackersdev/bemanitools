#define LOG_MODULE "p3ioemu"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "hook/iohook.h"

#include "iface-core/log.h"

#include "p3io/cmd.h"
#include "p3io/frame.h"
#include "p3io/ioctl.h"

#include "p3ioemu/devmgr.h"
#include "p3ioemu/emu.h"
#include "p3ioemu/uart.h"

#include "util/iobuf.h"

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
    const struct p3io_req_version *req, struct p3io_resp_version *resp);
static void p3io_cmd_set_watchdog(
    const struct p3io_req_watchdog *req, struct p3io_resp_watchdog *resp);
static void p3io_cmd_set_outputs(
    const struct p3io_req_set_outputs *req, struct p3io_resp_set_outputs *resp);
static void p3io_cmd_read_plug(
    const struct p3io_req_read_plug *req, struct p3io_resp_read_plug *resp);
static void p3io_cmd_get_cab_type_or_dipsw(
    const struct p3io_req_get_cab_type_or_dipsw *req,
    struct p3io_resp_get_cab_type_or_dipsw *resp);
static void p3io_cmd_get_video_freq(
    const struct p3io_req_get_video_freq *req,
    struct p3io_resp_get_video_freq *resp);
static void p3io_cmd_unknown_2b(
    const struct p3io_req_unknown_2b *req, struct p3io_resp_unknown_2b *resp);
static void
p3io_cmd_init(const struct p3io_req_init *req, struct p3io_resp_init *resp);
static void p3io_cmd_get_coinstock(
    const struct p3io_req_coin_stock *req, struct p3io_resp_coin_stock *resp);
static void p3io_cmd_set_coin_counter(
    const struct p3io_req_set_coin_counter *req,
    struct p3io_resp_set_coin_counter *resp);
static void p3io_cmd_unknown(
    const struct p3io_req_unknown_generic *req,
    struct p3io_resp_unknown_generic *resp);

void p3io_emu_init(const struct p3io_ops *ops, void *ctx)
{
    log_assert(p3io_emu_fd == NULL);
    log_assert(ops != NULL);

    HRESULT hr;

    p3io_emu_resp.bytes = p3io_emu_resp_bytes;
    p3io_emu_resp.nbytes = sizeof(p3io_emu_resp_bytes);
    p3io_emu_resp.pos = 0;

    p3io_ops = ops;
    p3io_ops_ctx = ctx;

    hr = iohook_open_nul_fd(&p3io_emu_fd);

    if (hr != S_OK) {
        log_fatal("Opening nul fd failed: %08lx", hr);
    }
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
        return iohook_invoke_next(irp);
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
        return iohook_invoke_next(irp);
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

    switch (irp->ioctl) {
        case P3IO_IOCTL_GET_VERSION: {
            const char dev_name[] = "bemanitools p3ioemu";

            if (irp->read.nbytes < strlen(dev_name)) {
                log_fatal("Device name string does not fit into buffer");
            }

            memset(irp->read.bytes, 0, irp->read.nbytes);
            memcpy(irp->read.bytes, dev_name, strlen(dev_name));
            irp->read.pos = strlen(dev_name);

            return S_OK;
        }

        case P3IO_IOCTL_RESET_PIPE:
            // does not expect a response
            irp->read.pos = 0;
            return S_OK;

        case P3IO_IOCTL_READ_JAMMA:
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

        default:
            log_warning("Unknown ioctl %08x", irp->ioctl);

            return E_NOTIMPL;
    }
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

    deframe.bytes = req.raw.data;
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

    switch (req->hdr.cmd) {
        case P3IO_CMD_GET_VERSION:
            p3io_cmd_get_version(&req->version, &resp.version);

            break;

        case P3IO_CMD_SET_WATCHDOG:
            p3io_cmd_set_watchdog(&req->watchdog, &resp.watchdog);

            break;

        case P3IO_CMD_SET_OUTPUTS:
            p3io_cmd_set_outputs(&req->set_outputs, &resp.set_outputs);

            break;

        case P3IO_CMD_READ_PLUG:
            p3io_cmd_read_plug(&req->read_plug, &resp.read_plug);

            break;

        case P3IO_CMD_GET_CAB_TYPE_OR_DIPSW:
            p3io_cmd_get_cab_type_or_dipsw(
                &req->cab_type_or_dipsw, &resp.cab_type_or_dipsw);

            break;

        case P3IO_CMD_GET_VIDEO_FREQ:
            p3io_cmd_get_video_freq(&req->video_freq, &resp.video_freq);

            break;

        case P3IO_CMD_UNKNOWN_2B:
            p3io_cmd_unknown_2b(&req->unknown_2b, &resp.unknown_2b);

            break;

        case P3IO_CMD_INIT:
            p3io_cmd_init(&req->init, &resp.init);

            break;

        case P3IO_CMD_GET_COINSTOCK:
            p3io_cmd_get_coinstock(&req->coin_stock, &resp.coin_stock);

            break;

        case P3IO_CMD_SET_COINCOUNTER:
            p3io_cmd_set_coin_counter(
                &req->set_coin_counter, &resp.set_coin_counter);

            break;

        case P3IO_CMD_RS232_OPEN_CLOSE:
            p3io_uart_cmd_open_close(
                &req->rs232_open_close, &resp.rs232_open_close);

            break;

        case P3IO_CMD_RS232_READ:
            p3io_uart_cmd_read(&req->rs232_read, &resp.rs232_read);

            break;

        case P3IO_CMD_RS232_WRITE:
            p3io_uart_cmd_write(&req->rs232_write, &resp.rs232_write);

            break;

        default:
            p3io_cmd_unknown(&req->unknown_generic, &resp.unknown_generic);

            break;
    }

    p3io_frame_encode(&p3io_emu_resp, &resp, resp.hdr.nbytes + 1);

    return S_OK;
}

static void p3io_cmd_get_version(
    const struct p3io_req_version *req, struct p3io_resp_version *resp)
{
    log_misc("Getting version");

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    resp->str[0] = 'H';
    resp->str[1] = 'D';
    resp->str[2] = 'X';
    resp->str[3] = '\0';
    resp->major = 1;
    resp->minor = 2;
    // resp->patch = 3;
}

static void p3io_cmd_set_watchdog(
    const struct p3io_req_watchdog *req, struct p3io_resp_watchdog *resp)
{
    log_misc("Setting watchdog: %d", req->enable);

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    resp->state = 0;
}

static void p3io_cmd_set_outputs(
    const struct p3io_req_set_outputs *req, struct p3io_resp_set_outputs *resp)
{
    uint32_t outputs;
    HRESULT hr;

    if (p3io_ops->set_outputs != NULL) {
        outputs = _byteswap_ulong(req->outputs);
        hr = p3io_ops->set_outputs(p3io_ops_ctx, outputs);
    } else {
        hr = S_OK;
    }

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    if (FAILED(hr)) {
        log_warning("Setting outputs failed: %lX", hr);
    }

    resp->unkn_FF = 0xFF;
}

static void p3io_cmd_read_plug(
    const struct p3io_req_read_plug *req, struct p3io_resp_read_plug *resp)
{
    HRESULT hr;

    log_misc("Reading plug: %02x", req->flags);

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

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
}

static void p3io_cmd_get_cab_type_or_dipsw(
    const struct p3io_req_get_cab_type_or_dipsw *req,
    struct p3io_resp_get_cab_type_or_dipsw *resp)
{
    HRESULT hr;
    uint8_t dipsw;
    enum p3io_cab_type type;

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    hr = S_OK;

    if (req->cab_type_or_dipsw == P3IO_CAB_TYPE_SELECTOR) {
        if (p3io_ops->get_cab_type) {
            hr = p3io_ops->get_cab_type(p3io_ops_ctx, &type);

            if (hr == S_OK) {
                switch (type) {
                    case P3IO_CAB_TYPE_SD:
                        resp->state = P3IO_CAB_TYPE_SD;
                        break;

                    case P3IO_CAB_TYPE_HD:
                        resp->state = P3IO_CAB_TYPE_HD;
                        break;

                    default:
                        log_assert(false);
                        break;
                }
            } else {
                resp->state = 0;
            }
        }
    } else if (req->cab_type_or_dipsw == P3IO_DIP_SW_SELECTOR) {
        if (p3io_ops->get_dipsw) {
            hr = p3io_ops->get_dipsw(p3io_ops_ctx, &dipsw);

            if (hr == S_OK) {
                resp->state = dipsw;
            } else {
                resp->state = 0;
            }
        }
    } else {
        log_warning(
            "Unknown value for cab type or dipsw: %d", req->cab_type_or_dipsw);
    }
}

static void p3io_cmd_get_video_freq(
    const struct p3io_req_get_video_freq *req,
    struct p3io_resp_get_video_freq *resp)
{
    HRESULT hr;

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    if (p3io_ops->get_video_freq != NULL) {
        hr = p3io_ops->get_video_freq(
            p3io_ops_ctx, (enum p3io_video_freq *) &resp->video_freq);
    } else {
        hr = S_OK;
        resp->video_freq = P3IO_VIDEO_FREQ_31KHZ;
    }

    if (hr == S_OK) {
        switch (resp->video_freq) {
            case P3IO_VIDEO_FREQ_15KHZ:
                log_misc("%s: Returning 15 kHz", __func__);
                break;

            case P3IO_VIDEO_FREQ_31KHZ:
                log_misc("%s: Returning 31 kHz", __func__);
                break;

            default:
                log_assert(false);
                break;
        }
    } else {
        log_misc("%s: Returning error! (hr=%x)", __func__, (int) hr);
    }
}

static void p3io_cmd_unknown_2b(
    const struct p3io_req_unknown_2b *req, struct p3io_resp_unknown_2b *resp)
{
    log_misc("Unknown 2b");

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    resp->unknown = 0;
}

static void
p3io_cmd_init(const struct p3io_req_init *req, struct p3io_resp_init *resp)
{
    log_misc("Init");

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    resp->status = 0;
}

static void p3io_cmd_get_coinstock(
    const struct p3io_req_coin_stock *req, struct p3io_resp_coin_stock *resp)
{
    uint16_t slots[2];
    HRESULT hr;

    memset(slots, 0, sizeof(slots));

    if (p3io_ops->get_coinstock != NULL) {
        hr = p3io_ops->get_coinstock(p3io_ops_ctx, slots, lengthof(slots));
    } else {
        hr = S_OK;
    }

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    resp->error = FAILED(hr);
    resp->slots[0] = _byteswap_ushort(slots[0]);
    resp->slots[1] = _byteswap_ushort(slots[1]);
}

static void p3io_cmd_set_coin_counter(
    const struct p3io_req_set_coin_counter *req,
    struct p3io_resp_set_coin_counter *resp)
{
    log_misc(
        "Setting coin counter: %02x %02x",
        req->coin_counter[0],
        req->coin_counter[1]);

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    resp->status = 0;
}

static void p3io_cmd_unknown(
    const struct p3io_req_unknown_generic *req,
    struct p3io_resp_unknown_generic *resp)
{
    log_warning(
        "Unsupported P3IO command, sending default response (might not "
        "work/crash though): %02x",
        req->hdr.cmd);

    p3io_resp_hdr_init(&resp->hdr, sizeof(*resp), &req->hdr);

    // Not always applicable/correct as there are several commands not
    // responding with any data, but fine for the majority of (unsupported)
    // commands
    // Remark: This might also lead to unpredictable behaviour or crashes
    resp->unknown = 0;
}
