#ifndef HOOK_IOHOOK_H
#define HOOK_IOHOOK_H

#include <windows.h>

#include <stddef.h>
#include <stdint.h>

#include "util/iobuf.h"

enum irp_op {
    IRP_OP_OPEN,
    IRP_OP_CLOSE,
    IRP_OP_READ,
    IRP_OP_WRITE,
    IRP_OP_IOCTL,
    IRP_OP_FSYNC,
    IRP_OP_SEEK,
};

struct irp {
    enum irp_op op;
    size_t next_handler;
    HANDLE fd;
    OVERLAPPED *ovl;
    struct const_iobuf write;
    struct iobuf read;
    uint32_t ioctl;
    const wchar_t *open_filename;
    uint32_t open_access;
    uint32_t open_share;
    SECURITY_ATTRIBUTES *open_sa;
    uint32_t open_creation;
    uint32_t open_flags;
    HANDLE *open_tmpl;
    uint32_t seek_origin;
    int64_t seek_offset;
    uint64_t seek_pos;
};

typedef HRESULT (*irp_handler_t)(struct irp *irp);

void iohook_init(const irp_handler_t *handlers, size_t nhandlers);
HANDLE iohook_open_dummy_fd(void);
HRESULT irp_invoke_next(struct irp *irp);

#endif
