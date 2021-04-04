#define LOG_MODULE "jbhook-io"

#include <string.h>

#include "bemanitools/jbio.h"

#include "imports/avs.h"

#include "jbhook/io.h"

#include "p4io/cmd.h"

#include "util/log.h"

static void jbhook_io_jamma2_read(void *resp, uint32_t nbytes);
static uint32_t jbhook_command_handle(
    uint8_t cmd,
    const void *payload,
    uint32_t payload_len,
    void *resp,
    uint32_t resp_max_len);

static const struct p4ioemu_device_msg_hook jbhook_io_msg = {
    .jamma2_read = jbhook_io_jamma2_read,
    .command_handle = jbhook_command_handle,
    .roundplug_read_id = NULL,
    .roundplug_read_mem = NULL};

/*
    0:0 ???
    0:1 b2
    0:2 b6
    0:3 b10
    0:4 b14
    0:5 b1
    0:6 b5
    0:7 b9

    1:0 ???
    1:1 b4
    1:2 b8
    1:3 b12
    1:4 b16
    1:5 b3
    1:6 b7
    1:7 b11

    2:0 b13
    2:1 ???
    2:2 ???
    2:3 ???
    2:4 b15
    2:5 ???
    2:6 ???
    2:7 ???

    3:0 coin
    3:1 service
    3:2 ???
    3:3 ???
    3:4 test
    3:5 ???
    3:6 ???
    3:7 ???
*/
static const uint32_t jbhook_io_panel_mappings[] = {
    (1 << 5),
    (1 << 1),
    (1 << 13),
    (1 << 9),
    (1 << 6),
    (1 << 2),
    (1 << 14),
    (1 << 10),
    (1 << 7),
    (1 << 3),
    (1 << 15),
    (1 << 11),
    (1 << 16),
    (1 << 4),
    (1 << 20),
    (1 << 12),
};

static const uint32_t jbhook_io_sys_button_mappings[] = {
    (1 << 28),
    (1 << 25),
    (1 << 24),
};

static void jbhook_io_jamma2_read(void *resp, uint32_t nbytes)
{
    uint32_t *inputs = (uint32_t *) resp;
    uint16_t panels;
    uint8_t buttons;

    /* lower three bytes low active, highest byte high active */
    *inputs = 0x00FFFFFF;

    if (!jb_io_read_inputs()) {
        log_warning("Reading inputs from jbio failed");
        return;
    }

    panels = jb_io_get_panel_inputs();
    buttons = jb_io_get_sys_inputs();

    for (uint8_t i = 0; i < 16; i++) {
        if (panels & (1 << i)) {
            *inputs &= ~jbhook_io_panel_mappings[i];
        }
    }

    for (uint8_t i = 0; i < 3; i++) {
        if (buttons & (1 << i)) {
            *inputs |= jbhook_io_sys_button_mappings[i];
        }
    }
}

static uint32_t jbhook_command_handle(
    uint8_t cmd,
    const void *payload,
    uint32_t payload_len,
    void *resp,
    uint32_t resp_max_len)
{
    switch (cmd) {
        case P4IO_CMD_COINSTOCK: {
            // on is 0x00, off is either 0x10 or 0x20 depending on whether it's
            // during gameplay (0x10) or test menu (0x20). Both seem to have the
            // same effect
            jb_io_set_coin_blocker(*(uint8_t*)payload == 0x00);

            // this actually returns the coinstock, don't care for it
            memset(resp, 0, 4);

            return 4;
        }

        case P4IO_CMD_SET_PORTOUT: {
            const struct p4io_req_panel_mode *req =
                (const struct p4io_req_panel_mode *) payload;

            // always fallback to ALL if input is unknown
            enum jb_io_panel_mode mode = JB_IO_PANEL_MODE_ALL;

            if(req->is_single) {
                switch(req->mode) {
                    case 0x0001:
                        mode = JB_IO_PANEL_MODE_TOP_LEFT;
                        break;

                    case 0x0000:
                        mode = JB_IO_PANEL_MODE_TOP_RIGHT;
                        break;

                    case 0x0101:
                        mode = JB_IO_PANEL_MODE_BOTTOM_LEFT;
                        break;

                    case 0x0100:
                        mode = JB_IO_PANEL_MODE_BOTTOM_RIGHT;
                        break;

                    default:
                        mode = JB_IO_PANEL_MODE_ALL;
                        break;
                }
            }

            jb_io_set_panel_mode(mode);

            memset(resp, 0, 1);

            return 1;
        }

        default:
            return 0xFFFFFFFF;
    }
}

const struct p4ioemu_device_msg_hook *jbhook_io_init(void)
{
    return &jbhook_io_msg;
}
