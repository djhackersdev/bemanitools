#define LOG_MODULE "ezusb-iidx"

#include "ezusb-iidx/ezusb-iidx.h"

#include "ezusb/ezusbsys2.h"

#include "util/hex.h"
#include "util/log.h"
#include "util/time.h"

#include "msg.h"

//#define DEBUG_DUMP

/* For debugging */
#ifdef DEBUG_DUMP
static void ezusb_iidx_log_usb(
    const char *prefix,
    uint32_t ctl_code,
    const BULK_TRANSFER_CONTROL *ctl,
    uint32_t ctl_size,
    void *header,
    uint32_t header_bytes,
    void *data,
    uint32_t data_bytes)
{
    char header_str[4096];
    char data_str[4096];
    const char *ctl_code_str;

    switch (ctl_code) {
        case IOCTL_Ezusb_GET_DEVICE_DESCRIPTOR:
            ctl_code_str = "GET_DEVICE_DESCRIPTOR";
            break;

        case IOCTL_Ezusb_VENDOR_REQUEST:
            ctl_code_str = "VENDOR_REQUEST";
            break;

        case IOCTL_EZUSB_ANCHOR_DOWNLOAD:
            ctl_code_str = "ANCHOR_DOWNLOAD";
            break;

        case IOCTL_EZUSB_BULK_READ:
            if (ctl->pipeNum == EZUSB_IIDX_MSG_PIPE_INTERRUPT_IN) {
                ctl_code_str = "INT_READ";
            } else if (ctl->pipeNum == EZUSB_IIDX_MSG_PIPE_BULK_IN) {
                ctl_code_str = "BULK_READ";
            } else {
                ctl_code_str = "INVALID_READ";
            }

            break;

        case IOCTL_EZUSB_BULK_WRITE:
            if (ctl->pipeNum == EZUSB_IIDX_MSG_PIPE_INTERRUPT_OUT) {
                ctl_code_str = "INT_WRITE";
            } else if (ctl->pipeNum == EZUSB_IIDX_MSG_PIPE_BULK_OUT) {
                ctl_code_str = "BULK_WRITE";
            } else {
                ctl_code_str = "INVALID_WRITE";
            }

            break;

        default:
            ctl_code_str = "UNKNOWN";
            break;
    }

    hex_encode_uc(header, header_bytes, header_str, sizeof(header_str));
    hex_encode_uc(data, data_bytes, data_str, sizeof(data_str));

    log_misc(
        "[EZUSB DUMP %s][%s] header(%d) %s |||| data(%d) %s\n",
        prefix,
        ctl_code_str,
        header_bytes,
        header_str,
        data_bytes,
        data_str);
}
#endif

static BOOL ezusb_iidx_device_io_control_wrapper(
    HANDLE fd,
    uint32_t code,
    void *in_bytes,
    uint32_t in_nbytes,
    void *out_bytes,
    uint32_t out_nbytes,
    DWORD *out_returned,
    OVERLAPPED *ovl)
{
    BOOL ret;

#ifdef DEBUG_DUMP
    ezusb_iidx_log_usb(
        "BEFORE",
        code,
        in_bytes,
        in_nbytes,
        in_bytes,
        in_nbytes,
        out_bytes,
        out_nbytes);
#endif

    ret = DeviceIoControl(
        fd,
        code,
        in_bytes,
        in_nbytes,
        out_bytes,
        out_nbytes,
        out_returned,
        ovl);

#ifdef DEBUG_DUMP
    ezusb_iidx_log_usb(
        "AFTER",
        code,
        in_bytes,
        in_nbytes,
        in_bytes,
        in_nbytes,
        out_bytes,
        out_nbytes);
#endif

    return ret;
}

bool ezusb_iidx_ioctl(
    HANDLE handle,
    uint32_t code,
    void *in_bytes,
    uint32_t in_nbytes,
    void *out_bytes,
    uint32_t out_nbytes,
    uint32_t *out_returned)
{
    return DeviceIoControl(
        handle,
        code,
        in_bytes,
        in_nbytes,
        out_bytes,
        out_nbytes,
        (LPDWORD) out_returned,
        NULL);
}

bool ezusb_iidx_interrupt_read(
    HANDLE handle, struct ezusb_iidx_msg_interrupt_read_packet *packet)
{
    BULK_TRANSFER_CONTROL transfer;
    DWORD outpkt;

    transfer.pipeNum = EZUSB_IIDX_MSG_PIPE_INTERRUPT_IN;

    return ezusb_iidx_device_io_control_wrapper(
        handle,
        IOCTL_EZUSB_BULK_READ,
        &transfer,
        sizeof(transfer),
        packet,
        sizeof(struct ezusb_iidx_msg_interrupt_read_packet),
        &outpkt,
        NULL);
}

bool ezusb_iidx_interrupt_write(
    HANDLE handle, const struct ezusb_iidx_msg_interrupt_write_packet *packet)
{
    BULK_TRANSFER_CONTROL transfer;
    DWORD outpkt;

    transfer.pipeNum = EZUSB_IIDX_MSG_PIPE_INTERRUPT_OUT;

    return ezusb_iidx_device_io_control_wrapper(
        handle,
        IOCTL_EZUSB_BULK_WRITE,
        &transfer,
        sizeof(transfer),
        (void *) packet,
        sizeof(struct ezusb_iidx_msg_interrupt_write_packet),
        &outpkt,
        NULL);
}

bool ezusb_iidx_bulk_read(
    HANDLE handle, struct ezusb_iidx_msg_bulk_packet *packet)
{
    BULK_TRANSFER_CONTROL transfer;
    DWORD outpkt;

    transfer.pipeNum = EZUSB_IIDX_MSG_PIPE_BULK_IN;

    return ezusb_iidx_device_io_control_wrapper(
        handle,
        IOCTL_EZUSB_BULK_READ,
        &transfer,
        sizeof(transfer),
        packet,
        sizeof(struct ezusb_iidx_msg_bulk_packet),
        &outpkt,
        NULL);
}

bool ezusb_iidx_bulk_write(
    HANDLE handle, const struct ezusb_iidx_msg_bulk_packet *packet)
{
    BULK_TRANSFER_CONTROL transfer;
    DWORD outpkt;

    transfer.pipeNum = EZUSB_IIDX_MSG_PIPE_BULK_OUT;

    return ezusb_iidx_device_io_control_wrapper(
        handle,
        IOCTL_EZUSB_BULK_WRITE,
        &transfer,
        sizeof(transfer),
        (void *) packet,
        sizeof(struct ezusb_iidx_msg_bulk_packet),
        &outpkt,
        NULL);
}

bool ezusb_iidx_execute_cmd(
    HANDLE handle,
    uint8_t node,
    uint8_t cmd,
    uint8_t cmd_detail,
    uint8_t cmd_detail2,
    struct ezusb_iidx_msg_interrupt_read_packet *int_read_data)
{
    struct ezusb_iidx_msg_interrupt_write_packet int_write;

    memset(&int_write, 0, sizeof(struct ezusb_iidx_msg_interrupt_write_packet));

    int_write.node = node;
    int_write.cmd = cmd;
    int_write.cmd_detail[0] = cmd_detail;
    int_write.cmd_detail[1] = cmd_detail2;

    log_misc(
        "Execute cmd: %02X %02X %02X %02X", node, cmd, cmd_detail, cmd_detail2);

    if (!ezusb_iidx_interrupt_write(handle, &int_write)) {
        log_warning("Interrupt write failed");
        return false;
    }

    Sleep(10);

    if (!ezusb_iidx_interrupt_read(handle, int_read_data)) {
        log_warning("Interrupt read failed");
        return false;
    }

    log_misc("Cmd result: %02X", int_read_data->status);

    return true;
}

uint8_t ezusb_iidx_execute_cmd_retry(
    HANDLE handle,
    uint8_t node,
    uint8_t cmd,
    uint8_t cmd_detail,
    uint8_t cmd_detail2,
    uint8_t int_reads)
{
    struct ezusb_iidx_msg_interrupt_read_packet int_read;
    struct ezusb_iidx_msg_interrupt_write_packet int_write;

    memset(&int_write, 0, sizeof(struct ezusb_iidx_msg_interrupt_write_packet));

    int_write.node = node;
    int_write.cmd = cmd;
    int_write.cmd_detail[0] = cmd_detail;
    int_write.cmd_detail[1] = cmd_detail2;

    log_misc(
        "Execute cmd: %02X %02X %02X %02X", node, cmd, cmd_detail, cmd_detail2);

    if (!ezusb_iidx_interrupt_write(handle, &int_write)) {
        log_warning("Interrupt write failed");
        return 0xFF;
    }

    Sleep(10);

    for (uint8_t i = 0; i < int_reads; i++) {
        if (!ezusb_iidx_interrupt_read(handle, &int_read)) {
            log_warning("Interrupt read failed");
            return 0xFF;
        }
    }

    log_misc("Cmd result: %02X", int_read.status);

    return int_read.status;
}

bool ezusb_iidx_execute_cmd_timeout(
    HANDLE handle,
    uint8_t node,
    uint8_t cmd,
    uint8_t cmd_detail,
    uint8_t cmd_detail2,
    uint8_t expected_ret_code,
    uint32_t timeout_ms)
{
    struct ezusb_iidx_msg_interrupt_read_packet int_read;
    struct ezusb_iidx_msg_interrupt_write_packet int_write;

    memset(&int_write, 0, sizeof(struct ezusb_iidx_msg_interrupt_write_packet));

    int_write.node = node;
    int_write.cmd = cmd;
    int_write.cmd_detail[0] = cmd_detail;
    int_write.cmd_detail[1] = cmd_detail2;

    log_misc(
        "Execute cmd: %02X %02X %02X %02X", node, cmd, cmd_detail, cmd_detail2);

    if (!ezusb_iidx_interrupt_write(handle, &int_write)) {
        log_warning("Interrupt write failed");
        return false;
    }

    Sleep(10);

    uint64_t start = time_get_counter();

    while (time_get_elapsed_ms(time_get_counter() - start) < timeout_ms) {
        if (!ezusb_iidx_interrupt_read(handle, &int_read)) {
            log_warning("Interrupt read failed");
            return false;
        }

        if (int_read.status == expected_ret_code) {
            log_misc("Cmd result: %02X", expected_ret_code);
            return true;
        }

        Sleep(10);
    }

    log_warning("Cmd failed, result: %02X", int_read.status);

    return false;
}