#define LOG_MODULE "lcd"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <ntdef.h>
#include <devioctl.h>
#include <ntddser.h>
// clang-format on

#include <stdbool.h>
#include <string.h>
#include <wchar.h>

#include "hook/iohook.h"

#include "sdvxhook/lcd.h"

#include "util/hex.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static HRESULT lcd_open(struct irp *irp);
static HRESULT lcd_close(struct irp *irp);
static HRESULT lcd_write(struct irp *irp);

static HANDLE lcd_fd;

void lcd_init(void)
{
    log_assert(lcd_fd == NULL);

    HRESULT hr;
    
    hr = iohook_open_nul_fd(&lcd_fd);

    if (hr != S_OK) {
        log_fatal("Opening nul fd failed: %08lx", hr);
    }
}

void lcd_fini(void)
{
    if (lcd_fd != NULL) {
        CloseHandle(lcd_fd);
    }

    lcd_fd = NULL;
}

HRESULT
lcd_dispatch_irp(struct irp *irp)
{
    log_assert(irp != NULL);

    if (irp->op != IRP_OP_OPEN && irp->fd != lcd_fd) {
        return iohook_invoke_next(irp);
    }

    switch (irp->op) {
        case IRP_OP_OPEN:
            return lcd_open(irp);
        case IRP_OP_CLOSE:
            return lcd_close(irp);
        case IRP_OP_READ:
            return S_OK;
        case IRP_OP_WRITE:
            return lcd_write(irp);
        case IRP_OP_IOCTL:
            return S_OK;
        default:
            return E_NOTIMPL;
    }
}

static HRESULT lcd_open(struct irp *irp)
{
    log_assert(irp != NULL);

    if (!wstr_eq(irp->open_filename, L"COM1")) {
        return iohook_invoke_next(irp);
    }

    irp->fd = lcd_fd;
    log_info("\"LCD\" port opened");

    return S_OK;
}

static HRESULT lcd_close(struct irp *irp)
{
    log_info("\"LCD\" port closed");

    return S_OK;
}

static HRESULT lcd_write(struct irp *irp)
{
    char str[128];
    size_t nbytes;

    log_assert(irp != NULL);

    if (irp->write.nbytes < sizeof(str)) {
        nbytes = irp->write.nbytes;
    } else {
        nbytes = sizeof(str) - 1;
    }

    memcpy(str, irp->write.bytes, nbytes);
    str[nbytes] = '\0';

    log_misc("-> %s", str);

    irp->write.pos = irp->write.nbytes;

    return S_OK;
}
