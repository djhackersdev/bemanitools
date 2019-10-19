#include <usb100.h>
#include <windows.h>

#include <stdbool.h>
#include <stdio.h>

#include "ezusb2/cyioctl.h"

#include "ezusb2-iidx/msg.h"

#include "hook/table.h"

#include "util/defs.h"

#define IOCTL_VEND_USB_REQ 0x220024

static BOOL STDCALL my_DeviceIoControl(
    HANDLE fd,
    uint32_t code,
    void *in_bytes,
    uint32_t in_nbytes,
    void *out_bytes,
    uint32_t out_nbytes,
    uint32_t *out_returned,
    OVERLAPPED *ovl);

static BOOL(STDCALL *real_DeviceIoControl)(
    HANDLE fd,
    uint32_t code,
    void *in_bytes,
    uint32_t in_nbytes,
    void *out_bytes,
    uint32_t out_nbytes,
    uint32_t *out_returned,
    OVERLAPPED *ovl);

static struct hook_symbol exit_hook_syms[] = {
    {.name = "DeviceIoControl",
     .patch = my_DeviceIoControl,
     .link = (void *) &real_DeviceIoControl},
};

static bool interrupt_write(
    HANDLE handle, const struct ezusb2_iidx_msg_interrupt_write_packet *packet)
{
    SINGLE_TRANSFER usb_req;
    uint32_t outpkt;

    /* PIPE_INT_OUT */
    usb_req.SetupPacket.bmRequest = 0x01;

    return real_DeviceIoControl(
        handle,
        IOCTL_VEND_USB_REQ,
        &usb_req,
        sizeof(usb_req),
        (void *) packet,
        sizeof(struct ezusb2_iidx_msg_interrupt_write_packet),
        &outpkt,
        NULL);
}

static void turn_off_lights(HANDLE fd)
{
    struct ezusb2_iidx_msg_interrupt_write_packet data;

    memset(&data, 0, sizeof(data));

    for (uint8_t i = 0; i < 9; i++) {
        data.seg16[i] = ' ';
    }

    OutputDebugString("Switching off lights...\n");

    if (!interrupt_write(fd, &data)) {
        OutputDebugString("Switching off lights failed\n");
    }
}

static BOOL STDCALL my_DeviceIoControl(
    HANDLE fd,
    uint32_t code,
    void *in_bytes,
    uint32_t in_nbytes,
    void *out_bytes,
    uint32_t out_nbytes,
    uint32_t *out_returned,
    OVERLAPPED *ovl)
{
    BOOL result;

    /* Call real first to get the input data */
    result = real_DeviceIoControl(
        fd,
        code,
        in_bytes,
        in_nbytes,
        out_bytes,
        out_nbytes,
        out_returned,
        ovl);

    if (code == IOCTL_VEND_USB_REQ) {
        PSINGLE_TRANSFER usb_req = in_bytes;

        /* Interrupt in endpoint */
        if (usb_req->SetupPacket.bmRequest == 0x81) {
            struct ezusb2_iidx_msg_interrupt_read_packet *msg_resp =
                (struct ezusb2_iidx_msg_interrupt_read_packet
                     *) (out_bytes + sizeof(SINGLE_TRANSFER));

            uint32_t pad = ~msg_resp->inverted_pad;

            /* Start P1 + Start P2 + VEFX + Effect */
            if ((pad & 0x0F) == 0x0F) {
                OutputDebugString("Triggered exit hook\n");

                turn_off_lights(fd);

                OutputDebugString("Done exit hook\n");

                /* Don't use ExitProcess. This might result in deadlocks
                   on newer games which rely more on multi threading */
                HANDLE hnd;
                hnd = OpenProcess(
                    SYNCHRONIZE | PROCESS_TERMINATE,
                    TRUE,
                    GetCurrentProcessId());
                TerminateProcess(hnd, 0);
            }
        }
    }

    return result;
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        hook_table_apply(
            NULL, "kernel32.dll", exit_hook_syms, lengthof(exit_hook_syms));
    }

    return TRUE;
}
