#include <windows.h> /* Usermode API */

#include <devioctl.h>
#include <ntddser.h>
#include <ntdef.h> /* Kernel-mode API for ioctls */

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#include "acioemu/emu.h"
#include "acioemu/pipe.h"

#include "hook/iohook.h"

#include "util/log.h"
#include "util/str.h"

static HRESULT ac_io_emu_open(struct ac_io_emu *emu, struct irp *irp);
static HRESULT ac_io_emu_close(struct ac_io_emu *emu, struct irp *irp);
static HRESULT ac_io_emu_read(struct ac_io_emu *emu, struct irp *irp);
static HRESULT ac_io_emu_write(struct ac_io_emu *emu, struct irp *irp);
static HRESULT ac_io_emu_ioctl(struct ac_io_emu *emu, struct irp *irp);

void ac_io_emu_init(struct ac_io_emu *emu, const wchar_t *filename)
{
    log_assert(emu != NULL);
    log_assert(filename != NULL);

    memset(emu, 0, sizeof(*emu));
    emu->fd = iohook_open_dummy_fd();
    emu->wfilename = wstr_dup(filename);
    wstr_narrow(filename, &emu->filename);
    ac_io_in_init(&emu->in);
    ac_io_out_init(&emu->out);
}

void ac_io_emu_fini(struct ac_io_emu *emu)
{
    log_assert(emu != NULL);

    free(emu->filename);
    free(emu->wfilename);

    if (emu->fd != NULL) {
        CloseHandle(emu->fd);
    }

    memset(emu, 0, sizeof(*emu));
}

bool ac_io_emu_match_irp(const struct ac_io_emu *emu, const struct irp *irp)
{
    log_assert(emu != NULL);
    log_assert(irp != NULL);

    if (irp->op == IRP_OP_OPEN) {
        return wstr_eq(emu->wfilename, irp->open_filename);
    } else {
        return irp->fd == emu->fd;
    }
}

HRESULT
ac_io_emu_dispatch_irp(struct ac_io_emu *emu, struct irp *irp)
{
    log_assert(irp != NULL);

    switch (irp->op) {
        case IRP_OP_OPEN:
            return ac_io_emu_open(emu, irp);
        case IRP_OP_CLOSE:
            return ac_io_emu_close(emu, irp);
        case IRP_OP_READ:
            return ac_io_emu_read(emu, irp);
        case IRP_OP_WRITE:
            return ac_io_emu_write(emu, irp);
        case IRP_OP_IOCTL:
            return ac_io_emu_ioctl(emu, irp);
        case IRP_OP_FSYNC:
            return S_FALSE;
        default:
            return E_NOTIMPL;
    }
}

static HRESULT ac_io_emu_open(struct ac_io_emu *emu, struct irp *irp)
{
    irp->fd = emu->fd;
    log_info("%s: ACIO port opened", emu->filename);

    return S_FALSE;
}

static HRESULT ac_io_emu_close(struct ac_io_emu *emu, struct irp *irp)
{
    log_info("%s: ACIO port closed", emu->filename);

    return S_FALSE;
}

static HRESULT ac_io_emu_read(struct ac_io_emu *emu, struct irp *irp)
{
    ac_io_in_drain(&emu->in, &irp->read);

    return S_FALSE;
}

static HRESULT ac_io_emu_write(struct ac_io_emu *emu, struct irp *irp)
{
    const struct ac_io_message *msg;

    for (;;) {
        ac_io_out_supply(&emu->out, &irp->write);

        if (!ac_io_out_have_message(&emu->out)) {
            break;
        }

        msg = ac_io_out_get_message(&emu->out);

        if (msg != NULL) {
            break;
        }

        ac_io_in_supply(&emu->in, NULL, 0);
        ac_io_out_consume_message(&emu->out);
    }

    return ac_io_out_have_message(&emu->out) ? S_OK : S_FALSE;
}

static HRESULT ac_io_emu_ioctl(struct ac_io_emu *emu, struct irp *irp)
{
    SERIAL_STATUS *status;

    log_assert(irp != NULL);

    switch (irp->ioctl) {
        case IOCTL_SERIAL_GET_COMMSTATUS:
            if (irp->read.bytes == NULL) {
                log_warning(
                    "IOCTL_SERIAL_GET_COMMSTATUS: Output buffer is NULL");

                return E_INVALIDARG;
            }

            if (irp->read.nbytes < sizeof(*status)) {
                log_warning("IOCTL_SERIAL_GET_COMMSTATUS: Buffer is too small");

                return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }

            status = (SERIAL_STATUS *) irp->read.bytes;
            status->Errors = 0;
            status->AmountInInQueue = ac_io_in_is_msg_pending(&emu->in);

            irp->read.pos = sizeof(*status);

            return S_FALSE;

        default:
            return S_FALSE;
    }
}

const struct ac_io_message *ac_io_emu_request_peek(const struct ac_io_emu *emu)
{
    log_assert(emu != NULL);

    return ac_io_out_get_message(&emu->out);
}

void ac_io_emu_request_pop(struct ac_io_emu *emu)
{
    log_assert(emu != NULL);

    ac_io_out_consume_message(&emu->out);
}

void ac_io_emu_response_push(
    struct ac_io_emu *emu, const struct ac_io_message *resp, uint64_t delay_us)
{
    log_assert(emu != NULL);
    log_assert(resp != NULL);

    ac_io_in_supply(&emu->in, resp, delay_us);
}

void ac_io_emu_response_push_thunk(
    struct ac_io_emu *emu, ac_io_in_thunk_t thunk, void *ctx, uint64_t delay_us)
{
    log_assert(emu != NULL);
    log_assert(thunk != NULL);

    ac_io_in_supply_thunk(&emu->in, thunk, ctx, delay_us);
}
