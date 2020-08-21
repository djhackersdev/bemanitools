#pragma once

#include <windows.h>

#include <stddef.h>
#include <stdint.h>

#include "hook/iobuf.h"

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

typedef HRESULT (*iohook_fn_t)(struct irp *irp);

HANDLE iohook_open_dummy_fd(void)
#ifdef __GNUC__
__attribute__((deprecated("Use iohook_open_nul_fd instead")))
#endif
;

HRESULT iohook_open_nul_fd(HANDLE *fd);
HRESULT iohook_push_handler(iohook_fn_t fn);
HRESULT iohook_invoke_next(struct irp *irp);
