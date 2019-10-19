#define LOG_MODULE "ezusb2-emu-util"

#include <windows.h>

#include <setupapi.h>
#include <stdio.h>
#include <usb100.h>

#include "ezusb2/cyioctl.h"
#include "ezusb2/ezusb2.h"

#include "ezusb-emu/util.h"

#include "util/hex.h"
#include "util/log.h"

void ezusb2_emu_util_log_usb_msg(const char *prefix, const struct irp *irp)
{
    SINGLE_TRANSFER *usb_req;
    const char *ctl_code_str;
    char setup_packet_str[4096];
    char single_transfer_str[4096];
    char read_data_str[4096];
    char write_data_str[4096];

    switch (irp->ioctl) {
        // TODO use macros
        case IOCTL_ADAPT_SEND_EP0_CONTROL_TRANSFER:
            ctl_code_str = "EP0_CTRL";
            break;

        case IOCTL_ADAPT_SEND_NON_EP0_TRANSFER:
            ctl_code_str = "VENDOR";
            break;

        default:
            ctl_code_str = "UNKNOWN";
            break;
    }

    usb_req = (SINGLE_TRANSFER *) irp->write.bytes;

    sprintf(
        setup_packet_str,
        "bmRequest 0x%X, bRequest 0x%X, wValue 0x%X, "
        "wIndex 0x%X, wLength %d, ulTimeOut %ld",
        usb_req->SetupPacket.bmRequest,
        usb_req->SetupPacket.bRequest,
        usb_req->SetupPacket.wValue,
        usb_req->SetupPacket.wIndex,
        usb_req->SetupPacket.wLength,
        usb_req->SetupPacket.ulTimeOut);

    sprintf(
        single_transfer_str,
        "reserverd 0x%X, ucEndpointAddress 0x%X, "
        "NtStatus 0x%lX, UsbdStatus 0x%lX, IsoPacketOffset %ld, "
        "IsoPacketLength %ld, BufferOffset %ld, BufferLength %ld",
        usb_req->reserved,
        usb_req->ucEndpointAddress,
        usb_req->NtStatus,
        usb_req->UsbdStatus,
        usb_req->IsoPacketOffset,
        usb_req->IsoPacketLength,
        usb_req->BufferOffset,
        usb_req->BufferLength);

    hex_encode_uc(
        irp->read.bytes,
        irp->read.nbytes,
        read_data_str,
        sizeof(read_data_str));
    hex_encode_uc(
        irp->write.bytes,
        irp->write.nbytes,
        write_data_str,
        sizeof(write_data_str));

    log_warning(
        "[EZUSB DUMP %s][%s] ctl_code 0x%X, in_len %d, out_len %d ||| "
        "setup packet: %s ||| single transfer: %s ||| read data: %s ||| "
        "write data: %s",
        prefix,
        ctl_code_str,
        irp->ioctl,
        irp->read.nbytes,
        irp->write.nbytes,
        setup_packet_str,
        single_transfer_str,
        read_data_str,
        write_data_str);
}