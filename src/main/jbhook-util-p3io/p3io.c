#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/jbio.h"

#include "core/log.h"

#include "jbhook-util-p3io/p3io.h"

#include "p3ioemu/emu.h"
#include "p3ioemu/uart.h"

#include "security/rp-sign-key.h"
#include "security/rp3.h"

static HRESULT jbhook_p3io_read_jamma(void *ctx, uint32_t *state);
static HRESULT jbhook_p3io_get_roundplug(
    void *ctx, uint8_t plug_id, uint8_t *rom, uint8_t *eeprom);
static HRESULT jbhook_p3io_set_outputs(void *ctx, uint32_t state);

/*
    0:0 b13
    0:1 -
    0:2 b15
    0:3 -
    0:4 test
    0:5 coin
    0:6 service
    0:7 -

    1:0 -
    1:1 b4
    1:2 b8
    1:3 b12
    1:4 b16
    1:5 b3
    1:6 b7
    1:7 b11

    2:0 -
    2:1 b2
    2:2 b6
    2:3 b10
    2:4 b14
    2:5 b1
    2:6 b5
    2:7 b9

    3:0 -
    3:1 -
    3:2 -
    3:3 -
    3:4 -
    3:5 -
    3:6 -
    3:7 -
*/
static const uint32_t jbhook_p3io_panel_mappings[] = {
    (1 << 21),
    (1 << 17),
    (1 << 13),
    (1 << 9),
    (1 << 22),
    (1 << 18),
    (1 << 14),
    (1 << 10),
    (1 << 23),
    (1 << 19),
    (1 << 15),
    (1 << 11),
    (1 << 0),
    (1 << 20),
    (1 << 2),
    (1 << 12),
};

static const uint32_t jbhook_p3io_sys_button_mappings[] = {
    (1 << 4),
    (1 << 6),
    (1 << 5),
};

static struct security_mcode jbhook_p3io_mcode;
static struct security_id jbhook_p3io_pcbid;
static struct security_id jbhook_p3io_eamid;

static const struct p3io_ops p3io_ddr_ops = {
    .read_jamma = jbhook_p3io_read_jamma,
    .get_roundplug = jbhook_p3io_get_roundplug,
    .set_outputs = jbhook_p3io_set_outputs,
};

void jbhook_util_p3io_init(
    const struct security_mcode *mcode,
    const struct security_id *pcbid,
    const struct security_id *eamid)
{
    memcpy(&jbhook_p3io_mcode, mcode, sizeof(struct security_mcode));
    memcpy(&jbhook_p3io_pcbid, pcbid, sizeof(struct security_id));
    memcpy(&jbhook_p3io_eamid, eamid, sizeof(struct security_id));

    p3io_emu_init(&p3io_ddr_ops, NULL);
}

void jbhook_util_p3io_fini(void)
{
    p3io_emu_fini();
}

static HRESULT jbhook_p3io_read_jamma(void *ctx, uint32_t *state)
{
    uint32_t state_tmp;
    uint16_t panels;
    uint8_t buttons;

    log_assert(state != NULL);

    state_tmp = 0;

    if (!jb_io_read_inputs()) {
        log_warning("Reading inputs from jbio failed");
        return E_FAIL;
    }

    panels = jb_io_get_panel_inputs();
    buttons = jb_io_get_sys_inputs();

    for (uint8_t i = 0; i < 16; i++) {
        // panels are active-low
        if ((panels & (1 << i)) == 0) {
            state_tmp |= jbhook_p3io_panel_mappings[i];
        }
    }

    for (uint8_t i = 0; i < 2; i++) {
        // sys buttons are active-high
        if (buttons & (1 << i)) {
            state_tmp |= jbhook_p3io_sys_button_mappings[i];
        }
    }

    // Single write to external/game managed buffer to reduce risk for
    // inconsistent state/random input misfiring
    *state = state_tmp;

    return S_OK;
}

static HRESULT jbhook_p3io_get_roundplug(
    void *ctx, uint8_t plug_id, uint8_t *rom, uint8_t *eeprom)
{
    uint8_t rom_out[sizeof(jbhook_p3io_pcbid.id)];
    struct security_rp3_eeprom eeprom_out;

    log_assert(sizeof(jbhook_p3io_pcbid.id) == sizeof(jbhook_p3io_eamid.id));

    if (plug_id == 0) {
        /* black */
        memcpy(rom_out, jbhook_p3io_pcbid.id, sizeof(jbhook_p3io_pcbid.id));
        security_rp3_generate_signed_eeprom_data(
            SECURITY_RP_UTIL_RP_TYPE_BLACK,
            &security_rp_sign_key_black_gfdmv4,
            &jbhook_p3io_mcode,
            &jbhook_p3io_pcbid,
            &eeprom_out);
    } else {
        /* white */
        memcpy(rom_out, jbhook_p3io_eamid.id, sizeof(jbhook_p3io_eamid.id));
        security_rp3_generate_signed_eeprom_data(
            SECURITY_RP_UTIL_RP_TYPE_WHITE,
            &security_rp_sign_key_white_eamuse,
            &security_mcode_eamuse,
            &jbhook_p3io_eamid,
            &eeprom_out);
    }

    // Single write to external/game managed buffer to reduce risk for
    // inconsistent state
    memcpy(rom, &rom_out, sizeof(rom_out));
    memcpy(eeprom, &eeprom_out, sizeof(struct security_rp3_eeprom));

    return S_OK;
}

static HRESULT jbhook_p3io_set_outputs(void *ctx, uint32_t state)
{
    uint8_t p3io_panel = (state & 0x00FF00) >> 8;
    uint8_t p3io_coinblocker = (state & 0xFF0000) >> 16;

    enum jb_io_panel_mode panel_mode;

    switch (p3io_panel) {
        case 5:
            panel_mode = JB_IO_PANEL_MODE_TOP_LEFT;
            break;
        case 4:
            panel_mode = JB_IO_PANEL_MODE_TOP_RIGHT;
            break;
        case 6:
            panel_mode = JB_IO_PANEL_MODE_BOTTOM_RIGHT;
            break;
        case 7:
            panel_mode = JB_IO_PANEL_MODE_BOTTOM_LEFT;
            break;
        default:
            panel_mode = JB_IO_PANEL_MODE_ALL;
            break;
    }

    jb_io_set_panel_mode(panel_mode);

    // 0x40 has been seen to unblock, no other values observed
    jb_io_set_coin_blocker(p3io_coinblocker == 0x00);

    return S_OK;
}
