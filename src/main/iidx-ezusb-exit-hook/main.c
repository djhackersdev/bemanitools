#include <windows.h>
#include <usb100.h>

#include <stdbool.h>
#include <stdio.h>

#include "ezusb/ezusbsys2.h"

#include "ezusb-iidx/msg.h"
#include "ezusb-iidx/seg16-cmd.h"

#include "hook/table.h"

#include "util/defs.h"

static BOOL STDCALL my_DeviceIoControl(
        HANDLE fd, uint32_t code, void *in_bytes, uint32_t in_nbytes,
        void *out_bytes, uint32_t out_nbytes, uint32_t *out_returned,
        OVERLAPPED *ovl);

static BOOL (STDCALL *real_DeviceIoControl)(
        HANDLE fd, uint32_t code, void *in_bytes, uint32_t in_nbytes,
        void *out_bytes, uint32_t out_nbytes, uint32_t *out_returned,
        OVERLAPPED *ovl);

static struct hook_symbol iidxfx_hook_syms[] = {
    {
        .name       = "DeviceIoControl",
        .patch      = my_DeviceIoControl,
        .link       = (void *) &real_DeviceIoControl
    },
};

static bool interrupt_write(HANDLE handle,
        const struct ezusb_iidx_msg_interrupt_write_packet* packet)
{
    BULK_TRANSFER_CONTROL transfer;
    uint32_t outpkt;

    transfer.pipeNum = EZUSB_IIDX_MSG_PIPE_INTERRUPT_OUT;

    return real_DeviceIoControl(
        handle, IOCTL_EZUSB_BULK_WRITE, &transfer,
        sizeof(transfer), (void*) packet,
        sizeof(struct ezusb_iidx_msg_interrupt_write_packet), &outpkt, NULL);
}

static bool bulk_write(HANDLE handle,
        const struct ezusb_iidx_msg_bulk_packet* packet)
{
    BULK_TRANSFER_CONTROL transfer;
    uint32_t outpkt;

    transfer.pipeNum = EZUSB_IIDX_MSG_PIPE_BULK_OUT;

    return real_DeviceIoControl(
        handle, IOCTL_EZUSB_BULK_WRITE, &transfer,
        sizeof(transfer), (void*) packet,
        sizeof(struct ezusb_iidx_msg_bulk_packet), &outpkt, NULL);
}

static void turn_off_lights(HANDLE fd)
{
    struct ezusb_iidx_msg_interrupt_write_packet data;
    struct ezusb_iidx_msg_bulk_packet bulk;

    data.deck_lights = 0;
    data.node = EZUSB_IIDX_MSG_NODE_16SEG;
    data.cmd = EZUSB_IIDX_16SEG_CMD_WRITE;
    data.cmd_detail[0] = 0;
    data.cmd_detail[1] = 1;
    data.top_lamps = 0;
    data.top_neons = 0;
    data.fpga_run = 1;

    OutputDebugString("Switching off lights...\n");

    /* avoid crashing the IO because of short request bursts */
    Sleep(10);

    if (!interrupt_write(fd, &data)) {
        OutputDebugString("Switching off lights failed\n");
    }

    bulk.node = EZUSB_IIDX_MSG_NODE_16SEG;
    bulk.page = 0;
    memset(bulk.payload, ' ', sizeof(bulk.payload));

    /* avoid crashing the IO because of short request bursts */
    Sleep(10);

    if (!bulk_write(fd, &bulk)) {
        OutputDebugString("Switching off lights (16seg) failed\n");
    }
}

static BOOL STDCALL my_DeviceIoControl(
        HANDLE fd, uint32_t code, void *in_bytes, uint32_t in_nbytes,
        void *out_bytes, uint32_t out_nbytes, uint32_t *out_returned,
        OVERLAPPED *ovl)
{
    BOOL result;

    /* Call real first to get the input data */
    result = real_DeviceIoControl(fd, code, in_bytes, in_nbytes, out_bytes,
        out_nbytes, out_returned, ovl);

    if (code == IOCTL_EZUSB_BULK_READ) {
        const BULK_TRANSFER_CONTROL *ctl = in_bytes;

        /* Pipe Interrupt In */
        if (ctl->pipeNum == 1) {
            struct ezusb_iidx_msg_interrupt_read_packet* msg_resp =
                (struct ezusb_iidx_msg_interrupt_read_packet*) out_bytes;

            uint32_t pad = ~msg_resp->inverted_pad;

            /* First (invalid) data we get from a real C02 IO board. Must be
               filtered to avoid instant exit trigger on boot */
            if (pad != 0xFFFFFF00) {

                /* Start P1 + Start P2 + VEFX + Effect */
                if (((pad >> 24) & 0x0F) == 0x0F) {
                    OutputDebugString("Triggered exit hook\n");

                    turn_off_lights(fd);

                    OutputDebugString("Done exit hook\n");

                    Sleep(100);

                    /* Don't use ExitProcess. This might result in deadlocks
                       on newer games which rely more on multi threading */
                    HANDLE hnd;
                    hnd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE,
                        GetCurrentProcessId());
                    TerminateProcess(hnd, 0);
                }
            }
        }
    }

    return result;
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        hook_table_apply(
                NULL,
                "kernel32.dll",
                iidxfx_hook_syms,
                lengthof(iidxfx_hook_syms));

    }

    return TRUE;
}
