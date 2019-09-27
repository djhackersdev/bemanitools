#define LOG_MODULE "spike"

#include <windows.h>

#include <ntdef.h>
#include <devioctl.h>
#include <ntddser.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "acioemu/addr.h"
#include "acioemu/emu.h"

#include "hook/iohook.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"

static struct ac_io_emu spike_ac_io_emu;

static void spike_handle_broadcast(const struct ac_io_message *bcast);
static void spike_handle_get_version(const struct ac_io_message *req);
static void spike_handle_status(const struct ac_io_message *req);
static void spike_send_empty(const struct ac_io_message *req);

void spike_init(void)
{
    ac_io_emu_init(&spike_ac_io_emu, L"COM2");
}

void spike_fini(void)
{
    ac_io_emu_fini(&spike_ac_io_emu);
}

HRESULT spike_dispatch_irp(struct irp *irp)
{
    const struct ac_io_message *msg;
    HRESULT hr;

    log_assert(irp != NULL);

    if (!ac_io_emu_match_irp(&spike_ac_io_emu, irp)) {
        return irp_invoke_next(irp);
    }

    for (;;) {
        hr = ac_io_emu_dispatch_irp(&spike_ac_io_emu, irp);

        if (hr != S_OK) {
            return hr;
        }

        msg = ac_io_emu_request_peek(&spike_ac_io_emu);

        switch (msg->addr) {
        case 0:
            ac_io_emu_cmd_assign_addrs(&spike_ac_io_emu, msg, 7);

            break;

        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            switch (ac_io_u16(msg->cmd.code)) {
            case AC_IO_CMD_GET_VERSION:
                spike_handle_get_version(msg);

                break;

            case AC_IO_CMD_START_UP:
                spike_handle_status(msg);

                break;

            case AC_IO_CMD_KEEPALIVE:
                spike_send_empty(msg);

                break;

            case 0x100:
            case 0x110:
            case 0x112:
            case 0x128:
                spike_handle_status(msg);

                break;

            default:
                log_warning("Spike ACIO unhandled cmd: %04X",
                ac_io_u16(msg->cmd.code));
            }

            break;

        case AC_IO_BROADCAST:
            spike_handle_broadcast(msg);

            break;

        default:
            log_warning(
                    "Spike ACIO message on unhandled bus address: %d",
                    msg->addr);

            break;
        }

        ac_io_emu_request_pop(&spike_ac_io_emu);
    }
}

static void spike_handle_broadcast(const struct ac_io_message *bcast)
{
    /* The payload consists of 7x10 byte chunks. Chunk 0 seems to correspond
       to the front LED strip, and chunks 1-6 are the 2x3 spike bundles on the
       side of a dedicab. No idea about the format of this data other than
       that, try figuring it out if you're bored. */
}

static void spike_handle_get_version(const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.version);
    resp.cmd.version.type = ac_io_u32(AC_IO_NODE_TYPE_LED_SPIKE);
    resp.cmd.version.flag = 0x00;
    resp.cmd.version.major = 0x01;
    resp.cmd.version.minor = 0x01;
    resp.cmd.version.revision = 0x00;
    memcpy(resp.cmd.version.product_code, "DDRS",
            sizeof(resp.cmd.version.product_code));
    strncpy(resp.cmd.version.date, __DATE__, sizeof(resp.cmd.version.date));
    strncpy(resp.cmd.version.time, __TIME__, sizeof(resp.cmd.version.time));

    ac_io_emu_response_push(&spike_ac_io_emu, &resp, 0);
}

static void spike_handle_status(const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.status);
    resp.cmd.status = 0x00;

    ac_io_emu_response_push(&spike_ac_io_emu, &resp, 0);
}

static void spike_send_empty(const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = 0;

    ac_io_emu_response_push(&spike_ac_io_emu, &resp, 0);
}
