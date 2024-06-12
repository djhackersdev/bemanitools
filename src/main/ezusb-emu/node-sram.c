#define LOG_MODULE "ezusb-emu-node-sram"

#include <string.h>

#include "ezusb-emu/conf.h"
#include "ezusb-emu/node-sram.h"
#include "ezusb-iidx/sram-cmd.h"

#include "iface-core/log.h"

#include "util/fs.h"

#define SRAM_NPAGES 12

static uint8_t ezusb_iidx_emu_node_sram_buf[EZUSB_PAGESIZE * SRAM_NPAGES];
static enum ezusb_iidx_sram_command ezusb_iidx_emu_node_sram_last_cmd;
static int ezusb_iidx_emu_node_sram_read_page;

uint8_t ezusb_iidx_emu_node_sram_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    ezusb_iidx_emu_node_sram_last_cmd = cmd_id;

    switch (cmd_id) {
        case EZUSB_IIDX_SRAM_CMD_READ:
            log_misc("EZUSB_IIDX_SRAM_CMD_READ");
            ezusb_iidx_emu_node_sram_read_page = 0;
            break;

        case EZUSB_IIDX_SRAM_CMD_WRITE:
            log_misc("EZUSB_IIDX_SRAM_CMD_WRITE");
            ezusb_iidx_emu_node_sram_read_page = 0;
            break;

        case EZUSB_IIDX_SRAM_CMD_DONE:
            log_misc("EZUSB_IIDX_SRAM_CMD_DONE");

#ifdef EZUSB_IIDX_EMU_NODE_SRAM_DUMP
            file_save(
                "sram.bin",
                ezusb_iidx_emu_node_sram_buf,
                EZUSB_PAGESIZE * SRAM_NPAGES);

            log_info("Dumped sram data to sram.bin");
#endif

            break;

        default:
            log_warning("Unrecognised sram command: %02x", cmd_id);
            break;
    }

    return 0;
}

bool ezusb_iidx_emu_node_sram_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    if (ezusb_iidx_emu_node_sram_last_cmd == EZUSB_IIDX_SRAM_CMD_READ) {
        if (ezusb_iidx_emu_node_sram_read_page >= SRAM_NPAGES) {
            log_warning("SRAM read overrun");

            return false;
        }

        log_misc("Reading SRAM page %02x", ezusb_iidx_emu_node_sram_read_page);

        /* Gold to Sirius must have this set to get accepted */
        pkg->node = 0x40;
        pkg->page = (uint8_t) ezusb_iidx_emu_node_sram_read_page;
        memcpy(
            pkg->payload,
            ezusb_iidx_emu_node_sram_buf +
                ezusb_iidx_emu_node_sram_read_page * EZUSB_PAGESIZE,
            EZUSB_PAGESIZE);
        ezusb_iidx_emu_node_sram_read_page++;

        return true;

    } else {
        log_warning(
            "Unexpected SRAM read: ezusb_iidx_emu_node_sram_last_cmd = "
            "%02x",
            ezusb_iidx_emu_node_sram_last_cmd);

        return false;
    }
}

bool ezusb_iidx_emu_node_sram_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    if (pkg->page >= SRAM_NPAGES) {
        log_warning("SRAM write overrun");

        return false;
    }

    log_misc("Writing SRAM page %02x", pkg->page);
    memcpy(
        ezusb_iidx_emu_node_sram_buf + pkg->page * EZUSB_PAGESIZE,
        pkg->payload,
        EZUSB_PAGESIZE);

    return true;
}