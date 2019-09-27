#define LOG_MODULE "ezusb2-dbg"

#include <windows.h>
#include <usb100.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ezusb2/cyioctl.h"

#include "hook/table.h"

#include "util/cmdline.h"
#include "util/hex.h"
#include "util/log.h"
#include "util/str.h"

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

static BOOL STDCALL my_DeviceIoControl(
        HANDLE hFile,
        uint32_t dwIoControlCode,
        void *lpInBuffer,
        uint32_t nInBufferSize,
        void *lpOutBuffer,
        uint32_t nOutBufferSize,
        uint32_t *lpBytesReturned,
        OVERLAPPED *lpOverlapped);

static BOOL STDCALL my_CloseHandle(HANDLE fd);

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

static BOOL (STDCALL *real_CloseHandle)(HANDLE fd);

static struct hook_symbol ezusb2_dbg_hook_syms[] = {
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
        .name  = "DeviceIoControl",
        .patch = my_DeviceIoControl,
        .link  = (void *) &real_DeviceIoControl
    },
    {
        .name   = "CloseHandle",
        .patch  = my_CloseHandle,
        .link   = (void *) &real_CloseHandle,
    },
};

typedef void (*ezusb2_dbg_hook_handle_ioctl_t)(BOOL* result, HANDLE hFile,
        uint32_t dwIoControlCode,
        void *lpInBuffer,
        uint32_t nInBufferSize,
        void *lpOutBuffer,
        uint32_t nOutBufferSize,
        uint32_t *lpBytesReturned,
        OVERLAPPED *lpOverlapped);

static wchar_t ezusb2_dbg_hook_path[MAX_PATH];
static HANDLE ezusb2_dbg_hook_handle = INVALID_HANDLE_VALUE;
static ezusb2_dbg_hook_handle_ioctl_t ezusb2_dbg_hook_ioctl_handler;

static void ezusb2_dbg_hook_handle_ioctl_legacy(BOOL* result, HANDLE hFile,
        uint32_t dwIoControlCode, void *lpInBuffer, uint32_t nInBufferSize,
        void *lpOutBuffer, uint32_t nOutBufferSize, uint32_t *lpBytesReturned,
        OVERLAPPED *lpOverlapped)
{
    log_warning("TODO NOT IMPLEMENTED");
}

static void ezusb2_dbg_hook_handle_ioctl_fx2(BOOL* result, HANDLE hFile,
        uint32_t dwIoControlCode, void *lpInBuffer, uint32_t nInBufferSize,
        void *lpOutBuffer, uint32_t nOutBufferSize, uint32_t *lpBytesReturned,
        OVERLAPPED *lpOverlapped)
{
    char data_in[8192];
    char data_out[8192];

    if (nInBufferSize < sizeof(SINGLE_TRANSFER)) {
        log_warning("In buffer size, ioctl_code 0x%X too small: %d",
            dwIoControlCode, nInBufferSize);
    }

    if (nOutBufferSize < sizeof(SINGLE_TRANSFER)) {
        log_warning("Out buffer size, ioctl_code 0x%X too small: %d",
            dwIoControlCode, nOutBufferSize);
    }

    PSINGLE_TRANSFER trans_in = (PSINGLE_TRANSFER) lpInBuffer;
    PSINGLE_TRANSFER trans_out = (PSINGLE_TRANSFER) lpOutBuffer;

    memset(data_in, 0, sizeof(data_in));
    memset(data_out, 0, sizeof(data_out));

    hex_encode_uc(lpInBuffer + trans_in->BufferOffset, trans_in->BufferLength, 
        data_in, sizeof(data_in));
    hex_encode_uc(lpOutBuffer + trans_out->BufferOffset, 
        trans_out->BufferLength, data_out, sizeof(data_out));

    log_info("[ioctl][res %d, bytes returned %d, overlapped %d][ctrl_code 0x%X]"
        "[IN][bmRequest 0x%X, bRequest 0x%X, wValue 0x%X, wIndex 0x%X, "
        "wLength %d, ulTimeOut %ld][reserved 0x%X, ucEndpointAddress 0x%X, "
        "NtStatus 0x%lX, UsbdStatus 0x%lX, IsoPacketOffset %ld, "
        "IsoPacketLength %ld, BufferOffset %ld, BufferLength %ld][%s]"
        "[OUT][bmRequest 0x%X, bRequest 0x%X, wValue 0x%X, wIndex 0x%X, "
        "wLength %d, ulTimeOut %ld][reserved 0x%X, ucEndpointAddress 0x%X, "
        "NtStatus 0x%lX, UsbdStatus 0x%lX, IsoPacketOffset %ld, "
        "IsoPacketLength %ld, BufferOffset %ld, BufferLength %ld][%s]", 
        result ? *result : -1, result ? *lpBytesReturned : -1, 
        lpOverlapped ? 1 : 0, dwIoControlCode,
        trans_in->SetupPacket.bmRequest, trans_in->SetupPacket.bRequest,
        trans_in->SetupPacket.wValue, trans_in->SetupPacket.wIndex,
        trans_in->SetupPacket.wLength, trans_in->SetupPacket.ulTimeOut,
        trans_in->reserved, trans_in->ucEndpointAddress, trans_in->NtStatus,
        trans_in->UsbdStatus, trans_in->IsoPacketOffset, 
        trans_in->IsoPacketLength, trans_in->BufferOffset, 
        trans_in->BufferLength, data_in,
        trans_out->SetupPacket.bmRequest, trans_out->SetupPacket.bRequest,
        trans_out->SetupPacket.wValue, trans_out->SetupPacket.wIndex,
        trans_out->SetupPacket.wLength, trans_out->SetupPacket.ulTimeOut,
        trans_out->reserved, trans_out->ucEndpointAddress, trans_out->NtStatus,
        trans_out->UsbdStatus, trans_out->IsoPacketOffset,
        trans_out->IsoPacketLength, trans_out->BufferOffset,
        trans_out->BufferLength, data_out);
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
    char* file_name;
    HANDLE handle;

    if (lpFileName == NULL) {
        log_warning("%s: lpFileName == NULL", __func__);
        SetLastError(ERROR_INVALID_PARAMETER);

        return INVALID_HANDLE_VALUE;
    }

    wstr_narrow(lpFileName, &file_name);

    /* Don't spam log once we have our device */
    if (ezusb2_dbg_hook_handle == INVALID_HANDLE_VALUE) {
        log_misc("Opening: %s", file_name);
    }

    handle = real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, 
        lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, 
        hTemplateFile);

    if (!wcscmp(lpFileName, ezusb2_dbg_hook_path)) {
        log_info("Hit ezusb device, handle: %p", handle);

        if (ezusb2_dbg_hook_handle != INVALID_HANDLE_VALUE) {
            log_warning("Still active ezusb handle but opening new");
        }

        ezusb2_dbg_hook_handle = handle;
    }

    return handle;
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
    BOOL res;

    if (ezusb2_dbg_hook_handle != INVALID_HANDLE_VALUE && 
            hFile == ezusb2_dbg_hook_handle) {
        ezusb2_dbg_hook_ioctl_handler(NULL, hFile, dwIoControlCode, lpInBuffer, 
            nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, 
            lpOverlapped);
    }

    res = real_DeviceIoControl(hFile, dwIoControlCode, lpInBuffer, 
        nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, 
        lpOverlapped);

    if (ezusb2_dbg_hook_handle != INVALID_HANDLE_VALUE && 
            hFile == ezusb2_dbg_hook_handle) {
        ezusb2_dbg_hook_ioctl_handler(&res, hFile, dwIoControlCode, lpInBuffer, 
            nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, 
            lpOverlapped);
    }

    return res;
}

static BOOL STDCALL my_CloseHandle(HANDLE hFile)
{
    if (ezusb2_dbg_hook_handle == hFile) {
        log_info("Closing ezusb handle: %p", hFile);
        ezusb2_dbg_hook_handle = INVALID_HANDLE_VALUE;
    }

    return real_CloseHandle(hFile);
}

static void ezusb2_dbg_hook_terminate_process()
{
    /* Don't use ExitProcess. This might result in deadlocks
        on newer games which rely more on multi threading */
    HANDLE hnd;
    hnd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE,
        GetCurrentProcessId());
    TerminateProcess(hnd, 0);
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        FILE* file;
        int argc;
        char **argv;
        wchar_t* buffer;
        uint32_t args_success;

        file = fopen("ezusb2_dbg.log", "w+");
        log_to_writer(log_writer_file, file);

        hook_table_apply(
                NULL,
                "kernel32.dll",
                ezusb2_dbg_hook_syms,
                lengthof(ezusb2_dbg_hook_syms));

        if (real_CreateFileW == NULL) {
            /* my_CreateFileA requires this to be present */
            real_CreateFileW = (void *) GetProcAddress(
                    GetModuleHandleA("kernel32.dll"),
                    "CreateFileW");
        }

        args_recover(&argc, &argv);

        args_success = 0;

        for (int i = 0; i < argc; i++) {
            if (!strcmp(argv[i], "--ezusb2_dbg_path")) {
                if (i + 1 >= argc) {
                    continue;
                }

                buffer = str_widen(argv[i + 1]);

                wcscpy(ezusb2_dbg_hook_path, buffer);
                free(buffer);
                log_info("ezusb device path provided: %s", argv[i + 1]);
                args_success++;
            } else if (!strcmp(argv[i], "--ezusb2_dbg_type")) {
                if (i + 1 >= argc) {
                    continue;
                }

                int type = atoi(argv[i + 1]);

                switch (type) {
                    case 1:
                        ezusb2_dbg_hook_ioctl_handler = 
                            ezusb2_dbg_hook_handle_ioctl_legacy;
                        break;
                    case 2:
                        ezusb2_dbg_hook_ioctl_handler = 
                            ezusb2_dbg_hook_handle_ioctl_fx2;
                        break;
                    default:
                        break;
                }

                if (ezusb2_dbg_hook_ioctl_handler) {
                    log_info("ezusb type selected: %d", type);
                    args_success++;
                }
            }
        }

        if (args_success != 2) {
            log_fatal("No ezusb device path provided as arguments");
            log_fatal("ezusb2_dbg-hook usage: --ezusb_path <path>");
            ezusb2_dbg_hook_terminate_process();
        }

        args_free(argc, argv);
    }

    return TRUE;
}
