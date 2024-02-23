#define LOG_MODULE "ezusb-iidx-sram"

#include "core/log.h"

#include "ezusb-iidx/sram.h"

#include "ezusb-iidx.h"
#include "sram-cmd.h"

static bool ezusb_iidx_sram_write_page(
    HANDLE handle,
    uint8_t node,
    uint8_t page,
    const void *buffer,
    uint16_t size)
{
    struct ezusb_iidx_msg_bulk_packet packet;

    memset(&packet.payload, 0, 62);

    packet.node = node;
    packet.page = page;

    if (size > 62) {
        size = 62;
    }

    // log_misc("Writing sram page %d, size %d", page, size);

    memcpy(packet.payload, buffer, size);

    return ezusb_iidx_bulk_write(handle, &packet);
}

static bool ezusb_iidx_sram_read_page(HANDLE handle, uint8_t node)
{
    struct ezusb_iidx_msg_bulk_packet packet;

    memset(&packet, 0, sizeof(struct ezusb_iidx_msg_bulk_packet));

    return ezusb_iidx_bulk_read(handle, &packet);
}

static bool ezusb_iidx_sram_write(
    HANDLE handle,
    uint8_t node,
    const void *buffer,
    uint16_t size,
    uint8_t *pages_written)
{
    uint8_t page;
    uint16_t offset;

    page = 0;
    offset = 0;
    *pages_written = 0;

    /* Write in ezusb page sizes */
    while (offset < size) {
        if (!ezusb_iidx_sram_write_page(
                handle, node, page++, buffer + offset, size - offset)) {
            return false;
        }

        offset += 62;

        /* if we are too fast, the downloads might fail */
        Sleep(10);
        (*pages_written)++;
    }

    return true;
}

static bool ezusb_iidx_sram_read(HANDLE handle, uint8_t node, uint8_t pages)
{
    for (uint8_t i = 0; i < pages; i++) {
        if (!ezusb_iidx_sram_read_page(handle, node)) {
            return false;
        }

        /* if we are too fast, this might fail */
        Sleep(10);
    }

    return true;
}

bool ezusb_iidx_sram_init(HANDLE handle, const void *buffer, uint16_t size)
{
    uint8_t pages_written;
    struct ezusb_iidx_msg_interrupt_read_packet int_read_data;

    log_info("Init, size %d", size);

    ezusb_iidx_execute_cmd(
        handle,
        EZUSB_IIDX_MSG_NODE_SRAM,
        EZUSB_IIDX_SRAM_CMD_WRITE,
        1,
        0,
        &int_read_data);

    log_info("Writing...");

    if (!ezusb_iidx_sram_write(
            handle, EZUSB_IIDX_MSG_NODE_SRAM, buffer, size, &pages_written)) {
        log_warning(
            "Writing sram failed, pages written so far: %d", pages_written);
        return false;
    }

    log_info("Data written, pages: %d", pages_written);
    log_info("Reading back...");

    ezusb_iidx_execute_cmd(
        handle,
        EZUSB_IIDX_MSG_NODE_SRAM,
        EZUSB_IIDX_SRAM_CMD_READ,
        1,
        0,
        &int_read_data);

    if (!ezusb_iidx_sram_read(
            handle, EZUSB_IIDX_MSG_NODE_SRAM, pages_written)) {
        log_warning("Reading back failed");
        return false;
    }

    ezusb_iidx_execute_cmd(
        handle,
        EZUSB_IIDX_MSG_NODE_SRAM,
        EZUSB_IIDX_SRAM_CMD_DONE,
        1,
        0,
        &int_read_data);

    return true;
}