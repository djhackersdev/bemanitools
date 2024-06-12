#define LOG_MODULE "ezusb-iidx-emu-msg"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ezusb-emu/msg.h"
#include "ezusb-emu/node-coin.h"
#include "ezusb-emu/nodes.h"

#include "ezusb-iidx/msg.h"

#include "ezusb-iidx-emu/msg.h"
#include "ezusb-iidx-emu/node-serial.h"
#include "ezusb-iidx-emu/nodes.h"

#include "ezusb-iidx-16seg-emu/nodes.h"

#include "hook/iohook.h"

#include "iface-core/log.h"
#include "iface-io/iidx.h"

#include "util/hex.h"
/* ------------------------------------------------------------------------ */

static HRESULT ezusb_iidx_emu_msg_interrupt_read(struct iobuf *read);
static HRESULT ezusb_iidx_emu_msg_interrupt_write(struct const_iobuf *write);
static HRESULT ezusb_iidx_emu_msg_bulk_read(struct iobuf *read);
static HRESULT ezusb_iidx_emu_msg_bulk_write(struct const_iobuf *write);

/* ------------------------------------------------------------------------ */

static struct ezusb_emu_msg_hook ezusb_iidx_emu_msg_hook = {
    .interrupt_read = ezusb_iidx_emu_msg_interrupt_read,
    .interrupt_write = ezusb_iidx_emu_msg_interrupt_write,
    .bulk_read = ezusb_iidx_emu_msg_bulk_read,
    .bulk_write = ezusb_iidx_emu_msg_bulk_write};

/* ------------------------------------------------------------------------ */

static const struct ezusb_iidx_emu_node *ezusb_iidx_emu_msg_nodes[256] = {
    [EZUSB_IIDX_MSG_NODE_16SEG] = &ezusb_iidx_emu_node_16seg,
    [EZUSB_IIDX_MSG_NODE_COIN] = &ezusb_iidx_emu_node_coin,
    [EZUSB_IIDX_MSG_NODE_EEPROM] = &ezusb_iidx_emu_node_eeprom_v1,
    [EZUSB_IIDX_MSG_NODE_FPGA_V1] = &ezusb_iidx_emu_node_fpga_v1,
    [EZUSB_IIDX_MSG_NODE_NONE] = &ezusb_iidx_emu_node_none,
    [EZUSB_IIDX_MSG_NODE_SECURITY_MEM] = &ezusb_iidx_emu_node_security_mem_v1,
    [EZUSB_IIDX_MSG_NODE_SECURITY_PLUG] = &ezusb_iidx_emu_node_security_plug_v1,
    [EZUSB_IIDX_MSG_NODE_SERIAL] = &ezusb_iidx_emu_node_serial,
    [EZUSB_IIDX_MSG_NODE_SRAM] = &ezusb_iidx_emu_node_sram,
    [EZUSB_IIDX_MSG_NODE_WDT] = &ezusb_iidx_emu_node_wdt,
};

static const struct ezusb_iidx_emu_node *ezusb_iidx_emu_msg_v2_nodes[256] = {
    [EZUSB_IIDX_MSG_NODE_16SEG] = &ezusb_iidx_emu_node_16seg,
    [EZUSB_IIDX_MSG_NODE_COIN] = &ezusb_iidx_emu_node_coin,
    [EZUSB_IIDX_MSG_NODE_EEPROM] = &ezusb_iidx_emu_node_eeprom_v2,
    [EZUSB_IIDX_MSG_NODE_FPGA_V2] = &ezusb_iidx_emu_node_fpga_v2,
    [EZUSB_IIDX_MSG_NODE_NONE] = &ezusb_iidx_emu_node_none,
    [EZUSB_IIDX_MSG_NODE_SECURITY_MEM] = &ezusb_iidx_emu_node_security_mem_v2,
    [EZUSB_IIDX_MSG_NODE_SECURITY_PLUG] = &ezusb_iidx_emu_node_security_plug_v2,
    [EZUSB_IIDX_MSG_NODE_SRAM] = &ezusb_iidx_emu_node_sram,
    [EZUSB_IIDX_MSG_NODE_WDT] = &ezusb_iidx_emu_node_wdt,
};

static enum ezusb_iidx_emu_msg_io_board_type ezusb_iidx_emu_msg_io_board_type;
static const struct ezusb_iidx_emu_node **ezusb_iidx_emu_node_handler;
static uint8_t ezusb_iidx_emu_msg_status = 0;
static uint8_t ezusb_iidx_emu_msg_seq_no = 0;
static uint8_t ezusb_iidx_emu_msg_read_cur_node = 0;

/* ------------------------------------------------------------------------ */

struct ezusb_emu_msg_hook *
ezusb_iidx_emu_msg_init(enum ezusb_iidx_emu_msg_io_board_type io_board_type)
{
    if (io_board_type < 0 ||
        io_board_type >= EZUSB_IIDX_EMU_MSG_IO_BOARD_TYPE_COUNT) {
        log_fatal("Invalid io board type %d specified", io_board_type);
    }

    ezusb_iidx_emu_msg_io_board_type = io_board_type;

    /* Init all nodes */
    for (uint32_t i = 0; i < 256; i++) {
        /* "Constructor" optional */
        if (ezusb_iidx_emu_msg_nodes[i] &&
            ezusb_iidx_emu_msg_nodes[i]->init_node) {
            ezusb_iidx_emu_msg_nodes[i]->init_node();
        }
    }

    ezusb_iidx_emu_node_handler = ezusb_iidx_emu_msg_nodes;

    log_info(
        "Initialized, io board type: %d", ezusb_iidx_emu_msg_io_board_type);

    return &ezusb_iidx_emu_msg_hook;
}

struct ezusb_emu_msg_hook *ezusb_iidx_emu_msg_v2_init(void)
{
    /* Init all nodes */
    for (uint32_t i = 0; i < 256; i++) {
        /* "Constructor" optional */
        if (ezusb_iidx_emu_msg_v2_nodes[i] &&
            ezusb_iidx_emu_msg_v2_nodes[i]->init_node) {
            ezusb_iidx_emu_msg_v2_nodes[i]->init_node();
        }
    }

    ezusb_iidx_emu_node_handler = ezusb_iidx_emu_msg_v2_nodes;

    return &ezusb_iidx_emu_msg_hook;
}

static HRESULT ezusb_iidx_emu_msg_interrupt_read(struct iobuf *read)
{
    struct ezusb_iidx_msg_interrupt_read_packet *msg_resp =
        (struct ezusb_iidx_msg_interrupt_read_packet *) read->bytes;

    if (!bt_io_iidx_ep2_recv()) {
        return E_FAIL;
    }

    msg_resp->p1_turntable = bt_io_iidx_ep2_turntable_get(0);
    msg_resp->p2_turntable = bt_io_iidx_ep2_turntable_get(1);

    msg_resp->sliders[0] =
        bt_io_iidx_ep2_slider_get(0) | (bt_io_iidx_ep2_slider_get(1) << 4);

    msg_resp->sliders[1] =
        bt_io_iidx_ep2_slider_get(2) | (bt_io_iidx_ep2_slider_get(3) << 4);

    msg_resp->sliders[2] = bt_io_iidx_ep2_slider_get(4);

    msg_resp->inverted_pad = ((bt_io_iidx_ep2_keys_get() & 0x3FFF) << 8) |
        ((bt_io_iidx_ep2_panel_get() & 0x0F) << 24) |
        ((bt_io_iidx_ep2_sys_get() & 0x07) << 28) |
        (((bt_io_iidx_ep2_sys_get() >> 2) & 0x01) << 22);

    /* make sure to update the current coin mode state, otherwise
    the game will bang the IO and try to enforce the coin state it wants to
    set, which results in crashing the whole code here due to flooding it */
    msg_resp->inverted_pad &= ~(1 << 31);

    if (ezusb_iidx_emu_node_coin_get_mode() == 1) {
        msg_resp->inverted_pad |= (1 << 31);
    }

    msg_resp->inverted_pad = ~msg_resp->inverted_pad;

    msg_resp->status = ezusb_iidx_emu_msg_status;
    /* Reset status after delivered (important for eeprom reading) */
    ezusb_iidx_emu_msg_status = 0;

    msg_resp->seq_no = ezusb_iidx_emu_msg_seq_no++;
    msg_resp->fpga_write_ready = 1;

    /* serial io flags
    make sure flag is always cleared to pass on boot */
    msg_resp->serial_io_busy_flag = 0;

    if (ezusb_iidx_emu_node_serial_read_buffer_busy()) {
        msg_resp->serial_io_busy_flag |= (1 << 0);
    }

    if (ezusb_iidx_emu_node_serial_write_buffer_busy()) {
        msg_resp->serial_io_busy_flag |= (1 << 1);
    }

    /* this needs to be 2 with FPGA2_CMD_CHECK2,
    otherwise the game's fpga check will fail */
    msg_resp->fpga2_check_flag_unkn = 2;

    switch (ezusb_iidx_emu_msg_io_board_type) {
        case EZUSB_IIDX_EMU_MSG_IO_BOARD_TYPE_C02:
            // noop
            break;

        case EZUSB_IIDX_EMU_MSG_IO_BOARD_TYPE_D01:
            msg_resp->inverted_pad &= ~(1 << 4);
            break;

        case EZUSB_IIDX_EMU_MSG_IO_BOARD_TYPE_COUNT:
        default:
            log_fatal(
                "Illegal state, unhandled board type: %d",
                ezusb_iidx_emu_msg_io_board_type);
            break;
    }

    read->pos = sizeof(*msg_resp);

    return S_OK;
}

static HRESULT ezusb_iidx_emu_msg_interrupt_write(struct const_iobuf *write)
{
    const struct ezusb_iidx_msg_interrupt_write_packet *msg_req =
        (const struct ezusb_iidx_msg_interrupt_write_packet *) write->bytes;

    if (write->nbytes < sizeof(*msg_req)) {
        log_warning("Interrupt write message too small");

        return E_INVALIDARG;
    }

    bt_io_iidx_ep1_deck_lights_set(msg_req->deck_lights);
    bt_io_iidx_ep1_panel_lights_set(msg_req->panel_lights);
    bt_io_iidx_ep1_top_lamps_set(msg_req->top_lamps);
    bt_io_iidx_ep1_top_neons_set(msg_req->top_neons);

    if (!bt_io_iidx_ep1_send()) {
        return E_FAIL;
    }

    if (!ezusb_iidx_emu_node_handler[msg_req->node]) {
        ezusb_iidx_emu_msg_read_cur_node = 0;
        log_warning(
            "Unrecognised node in interrupt message: %02x", msg_req->node);

        return E_INVALIDARG;
    }

    /* Remember node for next bulk read */
    ezusb_iidx_emu_msg_read_cur_node = msg_req->node;
    ezusb_iidx_emu_msg_status =
        ezusb_iidx_emu_node_handler[msg_req->node]->process_cmd(
            msg_req->cmd, msg_req->cmd_detail[0], msg_req->cmd_detail[1]);

    return S_OK;
}

static HRESULT ezusb_iidx_emu_msg_bulk_read(struct iobuf *read)
{
    struct ezusb_iidx_msg_bulk_packet *pkt =
        (struct ezusb_iidx_msg_bulk_packet *) read->bytes;

    if (read->nbytes < sizeof(*pkt)) {
        log_warning("Bulk read buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    if (!ezusb_iidx_emu_node_handler[ezusb_iidx_emu_msg_read_cur_node]) {
        log_warning(
            "Bulk read unsupported on cur_node = %d",
            ezusb_iidx_emu_msg_read_cur_node);

        return E_FAIL;
    }

    if (!ezusb_iidx_emu_node_handler[ezusb_iidx_emu_msg_read_cur_node]
             ->read_packet(pkt)) {
        return E_FAIL;
    }

    read->pos = sizeof(*pkt);

    return S_OK;
}

static HRESULT ezusb_iidx_emu_msg_bulk_write(struct const_iobuf *write)
{
    const struct ezusb_iidx_msg_bulk_packet *pkt =
        (const struct ezusb_iidx_msg_bulk_packet *) write->bytes;

    if (write->nbytes < sizeof(*pkt)) {
        log_warning("Bulk write packet too small");

        return E_INVALIDARG;
    }

    if (!ezusb_iidx_emu_node_handler[pkt->node]) {
        log_warning("Bulk write not supported on pkt->node = %02x", pkt->node);

        return E_FAIL;
    }

    if (!ezusb_iidx_emu_node_handler[pkt->node]->write_packet(pkt)) {
        return E_FAIL;
    }

    return S_OK;
}
