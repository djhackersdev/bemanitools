#define LOG_MODULE "ezusb2-popn-emu-msg"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bemanitools/popnio.h"

#include "hook/iohook.h"

#include "ezusb-emu/msg.h"

#include "ezusb-iidx-emu/msg.h"
#include "ezusb-iidx-emu/nodes.h"
#include "ezusb-iidx-emu/node-none.h"
#include "ezusb-iidx-emu/node-coin.h"
#include "ezusb-iidx-emu/node-security-mem.h"
#include "ezusb-iidx-emu/node-security-plug.h"

#include "ezusb2-popn/msg.h"

#include "util/hex.h"
#include "util/log.h"

/* ------------------------------------------------------------------------ */

static HRESULT ezusb2_popn_emu_msg_interrupt_read(struct iobuf *read);
static HRESULT ezusb2_popn_emu_msg_interrupt_write(struct const_iobuf *write);
static HRESULT ezusb2_popn_emu_msg_bulk_read(struct iobuf *read);
static HRESULT ezusb2_popn_emu_msg_bulk_write(struct const_iobuf *write);

/* ------------------------------------------------------------------------ */

static struct ezusb_emu_msg_hook ezusb2_popn_emu_msg_hook = {
    .interrupt_read = ezusb2_popn_emu_msg_interrupt_read,
    .interrupt_write = ezusb2_popn_emu_msg_interrupt_write,
    .bulk_read = ezusb2_popn_emu_msg_bulk_read,
    .bulk_write = ezusb2_popn_emu_msg_bulk_write};

/* ------------------------------------------------------------------------ */

static const struct ezusb_iidx_emu_node *ezusb2_popn_emu_msg_nodes[256] = {
    [EZUSB_IIDX_MSG_NODE_COIN] = &ezusb_iidx_emu_node_coin,
    [EZUSB_IIDX_MSG_NODE_NONE] = &ezusb_iidx_emu_node_none,
    [EZUSB_IIDX_MSG_NODE_EEPROM] = &ezusb_iidx_emu_node_eeprom_v2,
    [EZUSB_IIDX_MSG_NODE_SECURITY_MEM] = &ezusb_iidx_emu_node_security_mem_v2,
    [EZUSB_IIDX_MSG_NODE_SECURITY_PLUG] = &ezusb_iidx_emu_node_security_plug_v2,
    [EZUSB_IIDX_MSG_NODE_SRAM] = &ezusb_iidx_emu_node_sram,
    [EZUSB_IIDX_MSG_NODE_WDT] = &ezusb_iidx_emu_node_wdt,
};

static uint8_t ezusb2_popn_emu_msg_status = 0;
static uint8_t ezusb2_popn_emu_msg_seq_no = 0;
static uint8_t ezusb2_popn_emu_msg_read_cur_node = 0;
static uint16_t ezusb2_popn_emu_msg_history[10];

/* ------------------------------------------------------------------------ */

struct ezusb_emu_msg_hook *ezusb2_popn_emu_msg_init(void)
{
    /* Init all nodes */
    for (uint32_t i = 0; i < 256; i++) {
        /* "Constructor" optional */
        if (ezusb2_popn_emu_msg_nodes[i] &&
            ezusb2_popn_emu_msg_nodes[i]->init_node) {
            ezusb2_popn_emu_msg_nodes[i]->init_node();
        }
    }

    memset(&ezusb2_popn_emu_msg_history[0], 0xff, sizeof(ezusb2_popn_emu_msg_history));

    return &ezusb2_popn_emu_msg_hook;
}

static HRESULT ezusb2_popn_emu_msg_interrupt_read(struct iobuf *read)
{
    struct ezusb2_popn_msg_interrupt_read_packet *msg_resp =
        (struct ezusb2_popn_msg_interrupt_read_packet *) read->bytes;

    memset(msg_resp, 0, sizeof(*msg_resp));

    msg_resp->unk0 = 0x03;
    msg_resp->unk1 = 0x1d;
    msg_resp->unk2 = 0x85;
    msg_resp->seq_no = ezusb2_popn_emu_msg_seq_no++;
    msg_resp->status = ezusb2_popn_emu_msg_status;
    msg_resp->unk3 = 0x00;
    msg_resp->coin_count = 0;
    msg_resp->unk4 = 0xfd;

    msg_resp->io.inverted_pad = ~popn_io_get_buttons();

    msg_resp->unk5 = 0x00;
    msg_resp->unk6 = 0x7d;
    msg_resp->unk7 = 0xdf;
    msg_resp->unk8 = ezusb2_popn_emu_msg_seq_no;

    memcpy(&msg_resp->button_history[0], &ezusb2_popn_emu_msg_history[0], sizeof(uint16_t) * 10);

    read->pos = sizeof(*msg_resp);

    memmove(&ezusb2_popn_emu_msg_history[1], &ezusb2_popn_emu_msg_history[0], sizeof(uint16_t) * 9);
    ezusb2_popn_emu_msg_history[0] = msg_resp->io.button;

    return S_OK;
}

static HRESULT ezusb2_popn_emu_msg_interrupt_write(struct const_iobuf *write)
{
    const struct ezusb2_popn_msg_interrupt_write_packet *msg_req =
        (const struct ezusb2_popn_msg_interrupt_write_packet *) write->bytes;

    if (write->nbytes < sizeof(*msg_req)) {
        log_warning("Interrupt write message too small");

        return E_INVALIDARG;
    }

    if (!ezusb2_popn_emu_msg_nodes[msg_req->node]) {
        ezusb2_popn_emu_msg_read_cur_node = 0;
        log_warning(
            "Unrecognised node in interrupt message: %02x", msg_req->node);

        return E_INVALIDARG;
    }

    popn_io_set_top_lights(msg_req->lamp & 0x1f);
    popn_io_set_side_lights((msg_req->lamp >> 8) & 0xf);
    popn_io_set_button_lights((msg_req->lamp >> 20) & 0xfff);
    popn_io_set_coin_counter_light(((msg_req->lamp >> 12) & 0xf) == 0);  // Active low
    popn_io_set_coin_blocker_light(((msg_req->lamp >> 16) & 0xf) == 0xf);

    /* Remember node for next bulk read */

    ezusb2_popn_emu_msg_read_cur_node = msg_req->node;
    ezusb2_popn_emu_msg_status =
        ezusb2_popn_emu_msg_nodes[msg_req->node]->process_cmd(
            msg_req->cmd, msg_req->cmd_detail[0], msg_req->cmd_detail[1]);

    return S_OK;
}

static HRESULT ezusb2_popn_emu_msg_bulk_read(struct iobuf *read)
{
    struct ezusb_iidx_msg_bulk_packet *pkt =
        (struct ezusb_iidx_msg_bulk_packet *) read->bytes;

    if (read->nbytes < sizeof(*pkt)) {
        log_warning("Bulk read buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    if (!ezusb2_popn_emu_msg_nodes[ezusb2_popn_emu_msg_read_cur_node]) {
        log_warning(
            "Bulk read unsupported on cur_node = %d",
            ezusb2_popn_emu_msg_read_cur_node);

        return E_NOTIMPL;
    }

    if (!ezusb2_popn_emu_msg_nodes[ezusb2_popn_emu_msg_read_cur_node]
             ->read_packet(pkt)) {
        return E_FAIL;
    }

    read->pos = sizeof(*pkt);

    return S_OK;
}

static HRESULT ezusb2_popn_emu_msg_bulk_write(struct const_iobuf *write)
{
    const struct ezusb_iidx_msg_bulk_packet *pkt =
        (const struct ezusb_iidx_msg_bulk_packet *) write->bytes;

    if (write->nbytes < sizeof(*pkt)) {
        log_warning("Bulk write packet too small");

        return E_INVALIDARG;
    }

    if (!ezusb2_popn_emu_msg_nodes[pkt->node]) {
        log_warning("Bulk write not supported on pkt->node = %02x", pkt->node);

        return E_NOTIMPL;
    }

    if (!ezusb2_popn_emu_msg_nodes[pkt->node]->write_packet(pkt)) {
        return E_FAIL;
    }

    return S_OK;
}
