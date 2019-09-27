#define LOG_MODULE "ezusb-iidx-fpga"

#include "ezusb-iidx/fpga.h"

#include <stdio.h>

#include "util/log.h"

#include "ezusb-iidx.h"
#include "fpga-cmd.h"

static bool ezusb_iidx_fpga_write_fw_page(HANDLE handle, uint8_t node,
        const void* buffer, uint16_t size)
{
    struct ezusb_iidx_msg_bulk_packet packet;

    memset(&packet.payload, 0, 62);

    packet.node = node;
    packet.page = 0;

    if (size > 62) {
        size = 62;
    }

    //log_misc("Writing fpga page, size %d", size);

    memcpy(packet.payload, buffer, size);

    return ezusb_iidx_bulk_write(handle, &packet);
}

static bool ezusb_iidx_fpga_write_fw(HANDLE handle, uint8_t node,
        const void* buffer, uint16_t size)
{
    uint16_t offset;

    offset = 0;

    log_misc("Writing fw, size %d...", size);

    /* Write in ezusb page sizes */
    while (offset < size) {
        if (    !ezusb_iidx_fpga_write_fw_page(handle, node, buffer + offset,
                size - offset)) {
            return false;
        }

        offset += 62;
    }

    log_misc("Writing fw finished");

    return true;
}

static bool ezusb_iidx_fpga_v1_reset(HANDLE handle)
{
    log_info("[v1] Reset");

    if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V1,
            EZUSB_IIDX_FPGA_CMD_V1_INIT, 0x41, 0,
            EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK_2,
            10000)) {
        log_warning("[v1] Reset failed");
        return false;
    }

    return true;
}

static bool ezusb_iidx_fpga_v1_write(HANDLE handle, const void* buffer, uint16_t size)
{
    log_info("[v1] Write firmware, size %d", size);

    /* Have a bunch of sleep between each command. If we try to send commands as fast as possible, the
       hardware fails executing commands and might also crash. */

    if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V1,
            EZUSB_IIDX_FPGA_CMD_V1_CHECK, 0x41, 0,
            EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK,
            10000)) {
        log_warning("[v1] Check failed");
        return false;
    }

    Sleep(10);

    if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V1,
            EZUSB_IIDX_FPGA_CMD_V1_WRITE, size >> 8, size,
            EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK,
            10000)) {
        log_warning("[v1] Write failed");
        return false;
    }

    Sleep(10);

    if (    !ezusb_iidx_fpga_write_fw(handle, EZUSB_IIDX_MSG_NODE_FPGA_V1,
            buffer, size)) {
        log_warning("[v1] Writing fw failed");
        return false;
    }

    Sleep(10);

    if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V1,
            EZUSB_IIDX_FPGA_CMD_V1_WRITE_DONE, size >> 8, size,
            EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK_2,
            10000)) {
        log_warning("[v1] Write done failed");
        return false;
    }

    Sleep(10);

        if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V1,
            EZUSB_IIDX_FPGA_CMD_V1_CHECK, size >> 8, size,
            EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK,
            10000)) {
        log_warning("[v1] Check failed");
        return false;
    }

    Sleep(10);

    if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V1,
            EZUSB_IIDX_FPGA_CMD_V1_CHECK_2, size >> 8, size,
            EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK_2,
            10000)) {
        log_warning("[v1] Start failed");
        return false;
    }

    return true;
}

static bool ezusb_iidx_fpga_v2_reset(HANDLE handle)
{
    log_info("[v2] Reset");

    if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V2,
            EZUSB_IIDX_FPGA_CMD_V2_INIT, 1, 0,
            EZUSB_IIDX_FPGA_CMD_STATUS_V2_INIT_OK,
            10000)) {
        log_warning("[v2] Reset failed");
        return false;
    }

    return true;
}

static bool ezusb_iidx_fpga_v2_write(HANDLE handle, const void* buffer, uint16_t size)
{
    log_info("[v2] Waiting for fpga ready to write...");

    /* poll until the device reports fpga write ready */
    while (true) {
        struct ezusb_iidx_msg_interrupt_read_packet int_read_data;

        if (    !ezusb_iidx_execute_cmd(handle, EZUSB_IIDX_MSG_NODE_FPGA_V2,
                EZUSB_IIDX_FPGA_CMD_V2_WRITE, size >> 8, size,
                &int_read_data)) {
            log_warning("[v2] Write failed");
            return false;
        }

        if (int_read_data.fpga_write_ready == 1) {
            break;
        }

        Sleep(10);
    }

    log_info("[v2] Write firmware, size %d", size);

    if (    !ezusb_iidx_fpga_write_fw(handle, EZUSB_IIDX_MSG_NODE_FPGA_V2,
            buffer, size)) {
        log_warning("[v2] Writing fw failed");
        return false;
    }

    if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V2,
            EZUSB_IIDX_FPGA_CMD_V2_WRITE_DONE, size >> 8, size,
            EZUSB_IIDX_FPGA_CMD_STATUS_V2_WRITE_OK,
            10000)) {
        log_warning("[v2] Write done failed");
        return false;
    }

    if (    !ezusb_iidx_execute_cmd_timeout(handle, EZUSB_IIDX_MSG_NODE_FPGA_V2,
            EZUSB_IIDX_FPGA_CMD_V2_CHECK, size >> 8, size,
            EZUSB_IIDX_FPGA_CMD_STATUS_V2_CHECK_OK,
            10000)) {
        log_warning("[v2] Start failed");
        return false;
    }

    return true;
}

static bool ezusb_iidx_fpga_run_prog(HANDLE handle)
{
    /* Run the fpga prog */
    struct ezusb_iidx_msg_interrupt_write_packet int_write;
    memset(&int_write, 0, sizeof(struct ezusb_iidx_msg_interrupt_write_packet));

    int_write.fpga_run = 1;

    if (!ezusb_iidx_interrupt_write(handle, &int_write)) {
        log_warning("Run fpga prog failed");
        return false;
    }

    return true;
}

bool ezusb_iidx_fpga_v1_init(HANDLE handle, const void* buffer, uint16_t size)
{
    if (!ezusb_iidx_fpga_v1_reset(handle)) {
        return false;
    }

    Sleep(10);

    if (!ezusb_iidx_fpga_v1_write(handle, buffer, size)) {
        return false;
    }

    Sleep(10);

    if (!ezusb_iidx_fpga_run_prog(handle)) {
        return false;
    }

    return true;
}

bool ezusb_iidx_fpga_v2_init(HANDLE handle, const void* buffer, uint16_t size)
{
    if (!ezusb_iidx_fpga_v2_reset(handle)) {
        return false;
    }

    Sleep(10);

    if (!ezusb_iidx_fpga_v2_write(handle, buffer, size)) {
        return false;
    }

    Sleep(10);

    if (!ezusb_iidx_fpga_run_prog(handle)) {
        return false;
    }

    return true;
}