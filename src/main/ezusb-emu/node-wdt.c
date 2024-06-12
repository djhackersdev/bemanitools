#define LOG_MODULE "ezusb-emu-node-wdt"

#include "ezusb-emu/node-wdt.h"
#include "ezusb-iidx/wdt-cmd.h"

#include "iface-core/log.h"

uint8_t ezusb_iidx_emu_node_wdt_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_WDT_CMD_INIT:
            log_misc("EZUSB_IIDX_WDT_CMD_INIT");
            return EZUSB_IIDX_WDT_CMD_STATUS_OK;

        default:
            log_warning("Unrecognised wdt command: %02x", cmd_id);
            return EZUSB_IIDX_WDT_CMD_STATUS_FAULT;
    }
}

bool ezusb_iidx_emu_node_wdt_read_packet(struct ezusb_iidx_msg_bulk_packet *pkg)
{
    return true;
}

bool ezusb_iidx_emu_node_wdt_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    return true;
}