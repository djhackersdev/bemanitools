#define LOG_MODULE "ezusb-iidx-emu-node-fpga"

#include <string.h>

#include "ezusb-iidx-emu/conf.h"
#include "ezusb-iidx-emu/node-fpga.h"
#include "ezusb-iidx/fpga-cmd.h"

#include "util/fs.h"
#include "util/log.h"

static uint16_t ezusb_iidx_emu_node_fpga_write_ptr;
static uint16_t ezusb_iidx_emu_node_fpga_prog_size;
static uint8_t ezusb_iidx_emu_node_fpga_mem[0xFFFF];

uint8_t ezusb_iidx_emu_node_fpga_v1_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_FPGA_CMD_V1_INIT:
            log_misc("EZUSB_IIDX_FPGA_CMD_V1_INIT");
            return EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK_2;

        case EZUSB_IIDX_FPGA_CMD_V1_CHECK:
            log_misc("EZUSB_IIDX_FPGA_CMD_V1_CHECK");
            return EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK;

        case EZUSB_IIDX_FPGA_CMD_V1_CHECK_2:
            log_misc("EZUSB_IIDX_FPGA_CMD_V1_CHECK_2");
            return EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK_2;

        case EZUSB_IIDX_FPGA_CMD_V1_WRITE:
            ezusb_iidx_emu_node_fpga_prog_size = (cmd_data << 8) | cmd_data2;
            log_misc(
                "EZUSB_IIDX_FPGA_CMD_V1_WRITE (prog size %04X bytes)",
                ezusb_iidx_emu_node_fpga_prog_size);
            ezusb_iidx_emu_node_fpga_write_ptr = 0;
            memset(
                ezusb_iidx_emu_node_fpga_mem,
                0xFF,
                sizeof(ezusb_iidx_emu_node_fpga_mem));
            return EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK;

        case EZUSB_IIDX_FPGA_CMD_V1_WRITE_DONE:
            log_misc("EZUSB_IIDX_FPGA_CMD_V1_WRITE_DONE");
#ifdef EZUSB_IIDX_EMU_NODE_FPGA_DUMP
            file_save(
                "fpga.bin",
                ezusb_iidx_emu_node_fpga_mem,
                ezusb_iidx_emu_node_fpga_prog_size);

            log_info("Dumped fpga firmware to fpga.bin");
#endif
            return EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK_2;

        default:
            log_warning("Unrecognised fpga v1 command: %02x", cmd_id);
            return EZUSB_IIDX_FPGA_CMD_STATUS_V1_FAULT;
    }
}

uint8_t ezusb_iidx_emu_node_fpga_v2_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_FPGA_CMD_V2_INIT:
            log_misc("EZUSB_IIDX_FPGA_CMD_V2_INIT");
            return EZUSB_IIDX_FPGA_CMD_STATUS_V2_INIT_OK;

        case EZUSB_IIDX_FPGA_CMD_V2_CHECK:
            log_misc("EZUSB_IIDX_FPGA_CMD_V2_CHECK");
            return EZUSB_IIDX_FPGA_CMD_STATUS_V2_CHECK_OK;

        case EZUSB_IIDX_FPGA_CMD_V2_WRITE:
            ezusb_iidx_emu_node_fpga_prog_size = (cmd_data << 8) | cmd_data2;
            log_misc(
                "EZUSB_IIDX_FPGA_CMD_V2_WRITE (%04X bytes)",
                ezusb_iidx_emu_node_fpga_prog_size);
            ezusb_iidx_emu_node_fpga_write_ptr = 0;
            memset(
                ezusb_iidx_emu_node_fpga_mem,
                0xFF,
                sizeof(ezusb_iidx_emu_node_fpga_mem));
            return EZUSB_IIDX_FPGA_CMD_STATUS_V2_WRITE_OK;

        case EZUSB_IIDX_FPGA_CMD_V2_WRITE_DONE:
            log_misc("EZUSB_IIDX_FPGA_CMD_V2_WRITE_DONE");
#ifdef EZUSB_IIDX_EMU_NODE_FPGA_DUMP
            file_save(
                "fpga.bin",
                ezusb_iidx_emu_node_fpga_mem,
                ezusb_iidx_emu_node_fpga_prog_size);

            log_info("Dumped fpga firmware to fpga.bin");
#endif
            return EZUSB_IIDX_FPGA_CMD_STATUS_V2_WRITE_OK;

        default:
            log_warning("Unrecognised fpga v2 command: %02x", cmd_id);
            return EZUSB_IIDX_FPGA_CMD_STATUS_V2_FAULT;
    }
}

bool ezusb_iidx_emu_node_fpga_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_misc("FPGA read packet");

    /* stub */
    pkg->node = 0x00;
    pkg->page = 0x00;
    memset(pkg->payload, 0x00, sizeof(pkg->payload));

    return true;
}

bool ezusb_iidx_emu_node_fpga_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_misc("FPGA write packet: %02X %02X", pkg->node, pkg->page);

    memcpy(
        ezusb_iidx_emu_node_fpga_mem + ezusb_iidx_emu_node_fpga_write_ptr,
        pkg->payload,
        EZUSB_PAGESIZE);
    ezusb_iidx_emu_node_fpga_write_ptr += EZUSB_PAGESIZE;

    return true;
}