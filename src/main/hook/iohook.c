#define LOG_MODULE "iohook"

#include <windows.h>

#include <stdlib.h>
#include <string.h>

#include "hook/iohook.h"
#include "hook/table.h"

#include "util/hr.h"
#include "util/log.h"
#include "util/str.h"

/* Helpers */

static BOOL iohook_overlapped_result(
        uint32_t *syncout,
        OVERLAPPED *ovl,
        uint32_t value);

static HRESULT irp_invoke_real(struct irp *irp);
static HRESULT irp_invoke_real_open(struct irp *irp);
static HRESULT irp_invoke_real_close(struct irp *irp);
static HRESULT irp_invoke_real_read(struct irp *irp);
static HRESULT irp_invoke_real_write(struct irp *irp);
static HRESULT irp_invoke_real_seek(struct irp *irp);
static HRESULT irp_invoke_real_fsync(struct irp *irp);
static HRESULT irp_invoke_real_ioctl(struct irp *irp);

/* API hooks */

static BOOL STDCALL my_CloseHandle(HANDLE fd);

static HANDLE STDCALL my_CreateFileW(
        const wchar_t *lpFileName,
        uint32_t dwDesiredAccess,
        uint32_t dwShareMode,
        SECURITY_ATTRIBUTES *lpSecurityAttributes,
        uint32_t dwCreationDisposition,
        uint32_t dwFlagsAndAttributes,
        HANDLE hTemplateFile);

static HANDLE STDCALL my_CreateFileA(
        const char *lpFileName,
        uint32_t dwDesiredAccess,
        uint32_t dwShareMode,
        SECURITY_ATTRIBUTES *lpSecurityAttributes,
        uint32_t dwCreationDisposition,
        uint32_t dwFlagsAndAttributes,
        HANDLE hTemplateFile);

static BOOL STDCALL my_ReadFile(
        HANDLE hFile,
        void *lpBuffer,
        uint32_t nNumberOfBytesToRead,
        uint32_t *lpNumberOfBytesRead,
        OVERLAPPED *lpOverlapped);

static BOOL STDCALL my_WriteFile(
        HANDLE hFile,
        const void *lpBuffer,
        uint32_t nNumberOfBytesToWrite,
        uint32_t *lpNumberOfBytesWritten,
        OVERLAPPED *lpOverlapped);

static DWORD STDCALL my_SetFilePointer(
        HANDLE hFile,
        int32_t lDistanceToMove,
        int32_t *lpDistanceToMoveHigh,
        uint32_t dwMoveMethod);

static BOOL STDCALL my_FlushFileBuffers(HANDLE hFile);

static BOOL STDCALL my_DeviceIoControl(
        HANDLE hFile,
        uint32_t dwIoControlCode,
        void *lpInBuffer,
        uint32_t nInBufferSize,
        void *lpOutBuffer,
        uint32_t nOutBufferSize,
        uint32_t *lpBytesReturned,
        OVERLAPPED *lpOverlapped);

/* Links */

static BOOL (STDCALL *real_CloseHandle)(HANDLE fd);

static HANDLE (STDCALL *real_CreateFileW)(
        const wchar_t *filename,
        uint32_t access,
        uint32_t share,
        SECURITY_ATTRIBUTES *sa,
        uint32_t creation,
        uint32_t flags,
        HANDLE tmpl);

static BOOL (STDCALL *real_DeviceIoControl)(
        HANDLE fd,
        uint32_t code,
        void *in_bytes,
        uint32_t in_nbytes,
        void *out_bytes,
        uint32_t out_nbytes,
        uint32_t *out_returned,
        OVERLAPPED *ovl);

static BOOL (STDCALL *real_ReadFile)(
        HANDLE fd,
        void *buf,
        uint32_t nbytes,
        uint32_t *nread,
        OVERLAPPED *ovl);

static BOOL (STDCALL *real_WriteFile)(
        HANDLE fd,
        const void *buf,
        uint32_t nbytes,
        uint32_t *nwrit,
        OVERLAPPED *ovl);

static DWORD (STDCALL *real_SetFilePointer)(
        HANDLE hFile,
        int32_t lDistanceToMove,
        int32_t *lpDistanceToMoveHigh,
        uint32_t dwMoveMethod);

static BOOL (STDCALL *real_FlushFileBuffers)(HANDLE fd);

/* Hook table */

static struct hook_symbol iohook_kernel32_syms[] = {
    /* Basic IO */

    {
        .name   = "CloseHandle",
        .patch  = my_CloseHandle,
        .link   = (void *) &real_CloseHandle,
    },
    {
        .name   = "CreateFileA",
        .patch  = my_CreateFileA,
    },
    {
        .name   = "CreateFileW",
        .patch  = my_CreateFileW,
        .link   = (void *) &real_CreateFileW,
    },
    {
        .name   = "DeviceIoControl",
        .patch  = my_DeviceIoControl,
        .link   = (void *) &real_DeviceIoControl,
    },
    {
        .name   = "ReadFile",
        .patch  = my_ReadFile,
        .link   = (void *) &real_ReadFile,
    },
    {
        .name   = "WriteFile",
        .patch  = my_WriteFile,
        .link   = (void *) &real_WriteFile,
    },
    {
        .name   = "SetFilePointer",
        .patch  = my_SetFilePointer,
        .link   = (void *) &real_SetFilePointer,
    },
    {
        .name   = "FlushFileBuffers",
        .patch  = my_FlushFileBuffers,
        .link   = (void *) &real_FlushFileBuffers,
    },
};

static const irp_handler_t irp_real_handlers[] = {
    [IRP_OP_OPEN]   = irp_invoke_real_open,
    [IRP_OP_CLOSE]  = irp_invoke_real_close,
    [IRP_OP_READ]   = irp_invoke_real_read,
    [IRP_OP_WRITE]  = irp_invoke_real_write,
    [IRP_OP_SEEK]   = irp_invoke_real_seek,
    [IRP_OP_FSYNC]  = irp_invoke_real_fsync,
    [IRP_OP_IOCTL]  = irp_invoke_real_ioctl,
};

static const irp_handler_t *iohook_handlers;
static size_t iohook_nhandlers;

void iohook_init(const irp_handler_t *handlers, size_t nhandlers)
{
    log_assert(handlers != NULL);
    log_assert(iohook_handlers == NULL);

    iohook_handlers = handlers;
    iohook_nhandlers = nhandlers;

    hook_table_apply(
            NULL,
            "kernel32.dll",
            iohook_kernel32_syms,
            lengthof(iohook_kernel32_syms));

    if (real_CreateFileW == NULL) {
        /* my_CreateFileA requires this to be present */
        real_CreateFileW = (void *) GetProcAddress(
                GetModuleHandleA("kernel32.dll"),
                "CreateFileW");
    }

    log_info("IO Hook subsystem initialized");
}

HANDLE iohook_open_dummy_fd(void)
{
    HANDLE fd;

    fd = real_CreateFileW(
            L"NUL",
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL);

    if (fd == INVALID_HANDLE_VALUE) {
        log_fatal("Failed to open dummy fd: %08x", (uint32_t) GetLastError());
    }

    return fd;
}

static BOOL iohook_overlapped_result(
        uint32_t *syncout,
        OVERLAPPED *ovl,
        uint32_t value)
{
    if (ovl != NULL) {
        ovl->Internal = 0; // (NTSTATUS) STATUS_SUCCESS
        ovl->InternalHigh = value;

        if (ovl->hEvent != NULL) {
            SetEvent(ovl->hEvent);
        }
    }

    if (syncout != NULL) {
        *syncout = value;
        SetLastError(ERROR_SUCCESS);

        return TRUE;
    } else {
        SetLastError(ERROR_IO_PENDING);

        return FALSE;
    }
}

HRESULT irp_invoke_next(struct irp *irp)
{
    irp_handler_t handler;
    HRESULT hr;

    log_assert(irp != NULL);
    log_assert(irp->next_handler <= iohook_nhandlers);

    if (irp->next_handler < iohook_nhandlers) {
        handler = iohook_handlers[irp->next_handler++];
        hr = handler(irp);

        if (FAILED(hr)) {
            irp->next_handler = (size_t) -1;
        }
    } else {
        irp->next_handler = (size_t) -1;
        hr = irp_invoke_real(irp);
    }

    return hr;
}

static HRESULT irp_invoke_real(struct irp *irp)
{
    irp_handler_t handler;

    log_assert(irp != NULL);
    log_assert(irp->op < lengthof(irp_real_handlers));

    handler = irp_real_handlers[irp->op];

    log_assert(handler != NULL);

    return handler(irp);
}

static HRESULT irp_invoke_real_open(struct irp *irp)
{
    HANDLE fd;

    log_assert(irp != NULL);

    fd = real_CreateFileW(
            irp->open_filename,
            irp->open_access,
            irp->open_share,
            irp->open_sa,
            irp->open_creation,
            irp->open_flags,
            irp->open_tmpl);

    if (fd == INVALID_HANDLE_VALUE) {
        return hr_from_win32();
    }

    irp->fd = fd;

    return S_OK;
}

static HRESULT irp_invoke_real_close(struct irp *irp)
{
    BOOL ok;

    log_assert(irp != NULL);

    ok = real_CloseHandle(irp->fd);

    if (!ok) {
        return hr_from_win32();
    }

    return S_OK;
}

static HRESULT irp_invoke_real_read(struct irp *irp)
{
    uint32_t nread;
    BOOL ok;

    log_assert(irp != NULL);

    ok = real_ReadFile(
            irp->fd,
            &irp->read.bytes[irp->read.pos],
            irp->read.nbytes - irp->read.pos,
            &nread,
            irp->ovl);

    if (!ok) {
        return hr_from_win32();
    }

    irp->read.pos += nread;

    return S_OK;
}

static HRESULT irp_invoke_real_write(struct irp *irp)
{
    uint32_t nwrit;
    BOOL ok;

    log_assert(irp != NULL);

    ok = real_WriteFile(
            irp->fd,
            &irp->write.bytes[irp->write.pos],
            irp->write.nbytes - irp->write.pos,
            &nwrit,
            irp->ovl);

    if (!ok) {
        return hr_from_win32();
    }

    irp->write.pos += nwrit;

    return S_OK;
}

static HRESULT irp_invoke_real_seek(struct irp *irp)
{
    int32_t hi;
    int32_t lo;
    HRESULT hr;

    log_assert(irp != NULL);

    hi = (uint32_t) (irp->seek_offset >> 32);
    lo = (uint32_t) (irp->seek_offset      );

    lo = real_SetFilePointer(irp->fd, (int32_t) lo, hi == 0 ? NULL : &hi, 
            irp->seek_origin);

    if (lo == INVALID_SET_FILE_POINTER) {
        hr = hr_from_win32();

        if (FAILED(hr)) {
            return hr;
        }
    }

    irp->seek_pos = (((uint64_t) hi) << 32) | ((uint32_t) lo);

    return S_OK;
}

static HRESULT irp_invoke_real_fsync(struct irp *irp)
{
    BOOL ok;

    log_assert(irp != NULL);

    ok = real_FlushFileBuffers(irp->fd);

    if (!ok) {
        return hr_from_win32();
    }

    return S_OK;
}

static HRESULT irp_invoke_real_ioctl(struct irp *irp)
{
    uint32_t nread;
    BOOL ok;

    log_assert(irp != NULL);

    /* ioctl in/out params tend to be structs, so it probably does not make
       sense to concatenate a synthetic result with a pass-through result in
       the same way as one might do with read/write. */

    log_assert(irp->write.pos == 0);
    log_assert(irp->read.pos == 0);

    ok = real_DeviceIoControl(
            irp->fd,
            irp->ioctl,
            (void *) irp->write.bytes, // Cast off const
            irp->write.nbytes,
            irp->read.bytes,
            irp->read.nbytes,
            &nread,
            irp->ovl);

    if (!ok) {
        return hr_from_win32();
    }

    irp->read.pos = nread;

    return S_OK;
}

static HANDLE STDCALL my_CreateFileA(
        const char *lpFileName,
        uint32_t dwDesiredAccess,
        uint32_t dwShareMode,
        SECURITY_ATTRIBUTES *lpSecurityAttributes,
        uint32_t dwCreationDisposition,
        uint32_t dwFlagsAndAttributes,
        HANDLE hTemplateFile)
{
    wchar_t *wfilename;
    HANDLE fd;

    if (lpFileName == NULL) {
        log_warning("%s: lpFileName == NULL", __func__);
        SetLastError(ERROR_INVALID_PARAMETER);

        return INVALID_HANDLE_VALUE;
    }

    wfilename = str_widen(lpFileName);
    fd = my_CreateFileW(
            wfilename,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition, dwFlagsAndAttributes,
            hTemplateFile);
    free(wfilename);

    return fd;
}

static HANDLE STDCALL my_CreateFileW(
        const wchar_t *lpFileName,
        uint32_t dwDesiredAccess,
        uint32_t dwShareMode,
        SECURITY_ATTRIBUTES *lpSecurityAttributes,
        uint32_t dwCreationDisposition,
        uint32_t dwFlagsAndAttributes,
        HANDLE hTemplateFile)
{
    struct irp irp;
    HRESULT hr;

    if (lpFileName == NULL) {
        log_warning("%s: lpFileName == NULL", __func__);
        SetLastError(ERROR_INVALID_PARAMETER);

        return INVALID_HANDLE_VALUE;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_OPEN;
    irp.fd = INVALID_HANDLE_VALUE;
    irp.open_filename = lpFileName;
    irp.open_access = dwDesiredAccess;
    irp.open_share = dwShareMode;
    irp.open_sa = lpSecurityAttributes;
    irp.open_creation = dwCreationDisposition;
    irp.open_flags = dwFlagsAndAttributes;
    irp.open_tmpl = hTemplateFile;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        return hr_propagate_win32(hr, INVALID_HANDLE_VALUE);
    }

    SetLastError(ERROR_SUCCESS);

    return irp.fd;
}

static BOOL STDCALL my_CloseHandle(HANDLE hFile)
{
    struct irp irp;
    HRESULT hr;

    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
        log_warning("%s: Invalid file descriptor %p", __func__, hFile);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_CLOSE;
    irp.fd = hFile;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        return hr_propagate_win32(hr, FALSE);
    }

    SetLastError(ERROR_SUCCESS);

    return TRUE;
}

static BOOL STDCALL my_ReadFile(
        HANDLE hFile,
        void *lpBuffer,
        uint32_t nNumberOfBytesToRead,
        uint32_t *lpNumberOfBytesRead,
        OVERLAPPED *lpOverlapped)
{
    struct irp irp;
    HRESULT hr;

    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
        log_warning("%s: Invalid file descriptor %p", __func__, hFile);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    if (lpBuffer == NULL) {
        log_warning("%s: lpBuffer == NULL", __func__);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    if (lpOverlapped == NULL) {
        if (lpNumberOfBytesRead == NULL) {
            log_warning(    "%s: lpNumberOfBytesRead must be supplied in "
                            "synchronous mode",
                    __func__);
            SetLastError(ERROR_INVALID_PARAMETER);

            return FALSE;
        }

        *lpNumberOfBytesRead = 0;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_READ;
    irp.fd = hFile;
    irp.ovl = lpOverlapped;
    irp.read.bytes = lpBuffer;
    irp.read.nbytes = nNumberOfBytesToRead;
    irp.read.pos = 0;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        return hr_propagate_win32(hr, FALSE);
    }

    log_assert(irp.read.pos <= irp.read.nbytes);

    return iohook_overlapped_result(
            lpNumberOfBytesRead,
            lpOverlapped,
            irp.read.pos);
}

static BOOL STDCALL my_WriteFile(
        HANDLE hFile,
        const void *lpBuffer,
        uint32_t nNumberOfBytesToWrite,
        uint32_t *lpNumberOfBytesWritten,
        OVERLAPPED *lpOverlapped)
{
    struct irp irp;
    HRESULT hr;

    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
        /* Don't log this because iidx14 calls WriteFile with a NULL  handle */
        // log_warning("%s: Invalid file descriptor %p", __func__, hFile);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    if (lpBuffer == NULL) {
        log_warning("%s: lpBuffer == NULL", __func__);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    if (lpOverlapped == NULL) {
        if (lpNumberOfBytesWritten == NULL) {
            log_warning(    "%s: lpNumberOfBytesWritten must be supplied in "
                            "synchronous mode",
                    __func__);
            SetLastError(ERROR_INVALID_PARAMETER);

            return FALSE;
        }

        *lpNumberOfBytesWritten = 0;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_WRITE;
    irp.fd = hFile;
    irp.ovl = lpOverlapped;
    irp.write.bytes = lpBuffer;
    irp.write.nbytes = nNumberOfBytesToWrite;
    irp.write.pos = 0;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        return hr_propagate_win32(hr, FALSE);
    }

    log_assert(irp.write.pos <= irp.write.nbytes);

    return iohook_overlapped_result(
            lpNumberOfBytesWritten,
            lpOverlapped,
            irp.write.pos);
}

static DWORD STDCALL my_SetFilePointer(
        HANDLE hFile,
        int32_t lDistanceToMove,
        int32_t *lpDistanceToMoveHigh,
        uint32_t dwMoveMethod)
{
    struct irp irp;
    HRESULT hr;

    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
        log_warning("%s: Invalid file descriptor %p", __func__, hFile);
        SetLastError(ERROR_INVALID_PARAMETER);

        return INVALID_SET_FILE_POINTER;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_SEEK;
    irp.fd = hFile;
    irp.seek_origin = dwMoveMethod;

    /* This is a clumsy API. In 32-bit mode lDistanceToMove is a signed 32-bit
       int, but in 64-bit mode it is a 32-bit UNsigned int. Care must be taken
       with sign-extension vs zero-extension here. */

    if (lpDistanceToMoveHigh != NULL) {
        irp.seek_offset =     ((( int64_t) *lpDistanceToMoveHigh) << 32)
                            |  ((uint64_t) lDistanceToMove);
    } else {
        irp.seek_offset = (int64_t) lDistanceToMove;
    }

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        return hr_propagate_win32(hr, INVALID_SET_FILE_POINTER);
    }

    SetLastError(ERROR_SUCCESS);

    if (lpDistanceToMoveHigh != NULL) {
        *lpDistanceToMoveHigh = irp.seek_pos >> 32;
    }

    return (DWORD) irp.seek_pos;
}

static BOOL STDCALL my_FlushFileBuffers(HANDLE hFile)
{
    struct irp irp;
    HRESULT hr;

    /* Some of the old games using acio (e.g. DistorteD) are calling
       FlushFileBuffers. If that call is not hooked, the game will write a bunch
       of 0 data to the device and stop doing any read/write calls after that */

    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
        log_warning("%s: Invalid file descriptor %p", __func__, hFile);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_FSYNC;
    irp.fd = hFile;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        return hr_propagate_win32(hr, FALSE);
    }

    SetLastError(ERROR_SUCCESS);

    return TRUE;
}

static BOOL STDCALL my_DeviceIoControl(
        HANDLE hFile,
        uint32_t dwIoControlCode,
        void *lpInBuffer,
        uint32_t nInBufferSize,
        void *lpOutBuffer,
        uint32_t nOutBufferSize,
        uint32_t *lpBytesReturned,
        OVERLAPPED *lpOverlapped)
{
    struct irp irp;
    HRESULT hr;

    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
        log_warning("%s: Invalid file descriptor %p", __func__, hFile);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    if (lpOverlapped == NULL) {
        if (lpBytesReturned == NULL) {
            log_warning(
                    "%s: lpBytesReturned must be supplied in synchronous mode",
                    __func__);
            SetLastError(ERROR_INVALID_PARAMETER);

            return FALSE;
        }

        *lpBytesReturned = 0;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = hFile;
    irp.ovl = lpOverlapped;
    irp.ioctl = dwIoControlCode;

    if (lpInBuffer != NULL) {
        irp.write.bytes = lpInBuffer;
        irp.write.nbytes = nInBufferSize;
    }

    if (lpOutBuffer != NULL) {
        irp.read.bytes = lpOutBuffer;
        irp.read.nbytes = nOutBufferSize;
    }

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        return hr_propagate_win32(hr, FALSE);
    }

    log_assert(irp.write.pos <= irp.write.nbytes);
    log_assert(irp.read.pos <= irp.read.nbytes);

    return iohook_overlapped_result(
            lpBytesReturned,
            lpOverlapped,
            irp.read.pos);
}

