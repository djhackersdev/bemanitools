#define LOG_MODULE "ezusb2-iidx-emu-msg"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bemanitools/iidxio.h"

#include "core/log.h"

#include "hook/iohook.h"

#include "ezusb-emu/msg.h"
#include "ezusb-emu/nodes.h"
#include "ezusb-iidx-16seg-emu/nodes.h"

#include "ezusb2-iidx/msg.h"

#include "util/hex.h"

/* ------------------------------------------------------------------------ */

static HRESULT ezusb2_iidx_emu_msg_interrupt_read(struct iobuf *read);
static HRESULT ezusb2_iidx_emu_msg_interrupt_write(struct const_iobuf *write);
static HRESULT ezusb2_iidx_emu_msg_bulk_read(struct iobuf *read);
static HRESULT ezusb2_iidx_emu_msg_bulk_write(struct const_iobuf *write);

/* ------------------------------------------------------------------------ */

static struct ezusb_emu_msg_hook ezusb2_iidx_emu_msg_hook = {
    .interrupt_read = ezusb2_iidx_emu_msg_interrupt_read,
    .interrupt_write = ezusb2_iidx_emu_msg_interrupt_write,
    .bulk_read = ezusb2_iidx_emu_msg_bulk_read,
    .bulk_write = ezusb2_iidx_emu_msg_bulk_write};

/* ------------------------------------------------------------------------ */

static const struct ezusb_iidx_emu_node *ezusb2_iidx_emu_msg_nodes[256] = {
    [EZUSB_IIDX_MSG_NODE_16SEG] = &ezusb_iidx_emu_node_16seg,
    [EZUSB_IIDX_MSG_NODE_COIN] = &ezusb_iidx_emu_node_coin,
    [EZUSB_IIDX_MSG_NODE_NONE] = &ezusb_iidx_emu_node_none,
    [EZUSB_IIDX_MSG_NODE_SECURITY_MEM] = &ezusb_iidx_emu_node_security_mem_v2,
    [EZUSB_IIDX_MSG_NODE_SECURITY_PLUG] = &ezusb_iidx_emu_node_security_plug_v2,
};

static uint8_t ezusb2_iidx_emu_msg_status = 0;
static uint8_t ezusb2_iidx_emu_msg_seq_no = 0;
static uint8_t ezusb2_iidx_emu_msg_read_cur_node = 0;

/* ------------------------------------------------------------------------ */

struct ezusb_emu_msg_hook *ezusb2_iidx_emu_msg_init(void)
{
    /* Init all nodes */
    for (uint32_t i = 0; i < 256; i++) {
        /* "Constructor" optional */
        if (ezusb2_iidx_emu_msg_nodes[i] &&
            ezusb2_iidx_emu_msg_nodes[i]->init_node) {
            ezusb2_iidx_emu_msg_nodes[i]->init_node();
        }
    }

    return &ezusb2_iidx_emu_msg_hook;
}

static HRESULT ezusb2_iidx_emu_msg_interrupt_read(struct iobuf *read)
{
    struct ezusb2_iidx_msg_interrupt_read_packet msg_resp;

    if (!iidx_io_ep2_recv()) {
        return E_FAIL;
    }

    memset(&msg_resp, 0, sizeof(msg_resp));

    msg_resp.p1_turntable = iidx_io_ep2_get_turntable(0);
    msg_resp.p2_turntable = iidx_io_ep2_get_turntable(1);

    msg_resp.sliders[0] =
        iidx_io_ep2_get_slider(0) | (iidx_io_ep2_get_slider(1) << 4);

    msg_resp.sliders[1] =
        iidx_io_ep2_get_slider(2) | (iidx_io_ep2_get_slider(3) << 4);

    msg_resp.sliders[2] = iidx_io_ep2_get_slider(4);

    msg_resp.inverted_pad = ((iidx_io_ep2_get_keys() & 0x3FFF) << 16) |
        (iidx_io_ep2_get_panel() & 0x0F) |
        ((iidx_io_ep2_get_sys() & 0x07) << 4) |
        (((iidx_io_ep2_get_sys() >> 2) & 0x01) << 30);

    msg_resp.inverted_pad = ~msg_resp.inverted_pad;

    msg_resp.status = ezusb2_iidx_emu_msg_status;
    /* Reset status after delivered (important for eeprom reading) */
    ezusb2_iidx_emu_msg_status = 0;

    msg_resp.seq_no = ezusb2_iidx_emu_msg_seq_no++;

    // Single write to external/game managed buffer to reduce risk for
    // inconsistent state
    memcpy(read->bytes, &msg_resp, sizeof(msg_resp));
    read->pos = sizeof(msg_resp);

    return S_OK;
}

static HRESULT ezusb2_iidx_emu_msg_interrupt_write(struct const_iobuf *write)
{
    struct ezusb2_iidx_msg_interrupt_write_packet msg_req;

    if (write->nbytes < sizeof(msg_req)) {
        log_warning("Interrupt write message too small");

        return E_INVALIDARG;
    }

    // Single read from external/game managed buffer to reduce risk for
    // inconsistent state
    memcpy(&msg_req, write->bytes, sizeof(msg_req));

    if (!ezusb2_iidx_emu_msg_nodes[msg_req.node]) {
        ezusb2_iidx_emu_msg_read_cur_node = 0;
        log_warning(
            "Unrecognised node in interrupt message: %02x", msg_req.node);

        return E_INVALIDARG;
    }

    iidx_io_ep1_set_deck_lights(msg_req.deck_lights);
    iidx_io_ep1_set_panel_lights(msg_req.panel_lights);
    iidx_io_ep1_set_top_lamps(msg_req.top_lamps);
    iidx_io_ep1_set_top_neons(msg_req.top_neons);

    if (!iidx_io_ep1_send()) {
        return E_FAIL;
    }

    /* 16seg data is provided with the request and not handled using a
       separate bulk endpoint like on the C02 IO board */

    if (!iidx_io_ep3_write_16seg((const char *) msg_req.seg16)) {
        return E_FAIL;
    }

    /* Remember node for next bulk read */

    ezusb2_iidx_emu_msg_read_cur_node = msg_req.node;
    ezusb2_iidx_emu_msg_status =
        ezusb2_iidx_emu_msg_nodes[msg_req.node]->process_cmd(
            msg_req.cmd, msg_req.cmd_detail[0], msg_req.cmd_detail[1]);

    return S_OK;
}

static HRESULT ezusb2_iidx_emu_msg_bulk_read(struct iobuf *read)
{
    struct ezusb_iidx_msg_bulk_packet pkt;

    if (read->nbytes < sizeof(pkt)) {
        log_warning("Bulk read buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    memset(&pkt, 0, sizeof(pkt));

    if (!ezusb2_iidx_emu_msg_nodes[ezusb2_iidx_emu_msg_read_cur_node]) {
        log_warning(
            "Bulk read unsupported on cur_node = %d",
            ezusb2_iidx_emu_msg_read_cur_node);

        return E_NOTIMPL;
    }

    if (!ezusb2_iidx_emu_msg_nodes[ezusb2_iidx_emu_msg_read_cur_node]
             ->read_packet(&pkt)) {
        return E_FAIL;
    }

    // Single write to external/game managed buffer to reduce risk for
    // inconsistent state
    memcpy(read->bytes, &pkt, sizeof(pkt));
    read->pos = sizeof(pkt);

    return S_OK;
}

static HRESULT ezusb2_iidx_emu_msg_bulk_write(struct const_iobuf *write)
{
    struct ezusb_iidx_msg_bulk_packet pkt;

    if (write->nbytes < sizeof(pkt)) {
        log_warning("Bulk write packet too small");

        return E_INVALIDARG;
    }

    // Single read from external/game managed buffer to reduce risk for
    // inconsistent state
    memcpy(&pkt, write->bytes, sizeof(pkt));

    if (!ezusb2_iidx_emu_msg_nodes[pkt.node]) {
        log_warning("Bulk write not supported on pkt->node = %02x", pkt.node);

        return E_NOTIMPL;
    }

    if (!ezusb2_iidx_emu_msg_nodes[pkt.node]->write_packet(&pkt)) {
        return E_FAIL;
    }

    return S_OK;
}
