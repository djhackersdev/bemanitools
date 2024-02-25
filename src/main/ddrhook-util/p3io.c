#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/ddrio.h"

#include "ddrhook-util/p3io.h"

#include "core/log.h"

#include "p3ioemu/emu.h"
#include "p3ioemu/uart.h"

extern bool _15khz;
extern bool standard_def;

static HRESULT p3io_ddr_read_jamma(void *ctx, uint32_t *state);
static HRESULT p3io_ddr_set_outputs(void *ctx, uint32_t outputs);
static HRESULT p3io_ddr_get_cab_type(void *ctx, enum p3io_cab_type *type);
static HRESULT p3io_ddr_get_video_freq(void *ctx, enum p3io_video_freq *freq);
static HRESULT p3io_ddr_get_roundplug(
    void *ctx, uint8_t plug_id, uint8_t *rom, uint8_t *eeprom);

static const struct p3io_ops p3io_ddr_ops = {
    .read_jamma = p3io_ddr_read_jamma,
    .set_outputs = p3io_ddr_set_outputs,
    .get_cab_type = p3io_ddr_get_cab_type,
    .get_video_freq = p3io_ddr_get_video_freq,
};

static const struct p3io_ops p3io_ddr_ops_with_plugs = {
    .read_jamma = p3io_ddr_read_jamma,
    .set_outputs = p3io_ddr_set_outputs,
    .get_cab_type = p3io_ddr_get_cab_type,
    .get_video_freq = p3io_ddr_get_video_freq,
    .get_roundplug = p3io_ddr_get_roundplug,
};

static struct security_mcode p3io_ddr_mcode;
static struct security_id p3io_ddr_pcbid;
static struct security_id p3io_ddr_eamid;
static struct security_rp_sign_key p3io_black_sign_key;
static struct security_rp_sign_key p3io_white_sign_key;

void p3io_ddr_init(void)
{
    /* COM4 isn't a real COM port, we configure the core P3IO emulator code to
       generate IRPs addressed to COM4 and then we possibly intercept them in
       _com4.c (or possibly don't, in which case the communications will be
       sent to the real COM4 on your system) */

    p3io_uart_set_path(0, L"COM4");
    p3io_emu_init(&p3io_ddr_ops, NULL);
}

void p3io_ddr_init_with_plugs(
    const struct security_mcode *mcode,
    const struct security_id *pcbid,
    const struct security_id *eamid,
    const struct security_rp_sign_key *black_sign_key,
    const struct security_rp_sign_key *white_sign_key)
{
    memcpy(&p3io_ddr_mcode, mcode, sizeof(struct security_mcode));
    memcpy(&p3io_ddr_pcbid, pcbid, sizeof(struct security_id));
    memcpy(&p3io_ddr_eamid, eamid, sizeof(struct security_id));
    memcpy(
        &p3io_black_sign_key,
        black_sign_key,
        sizeof(struct security_rp_sign_key));
    memcpy(
        &p3io_white_sign_key,
        white_sign_key,
        sizeof(struct security_rp_sign_key));

    /* COM4 isn't a real COM port, we configure the core P3IO emulator code to
       generate IRPs addressed to COM4 and then we possibly intercept them in
       _com4.c (or possibly don't, in which case the communications will be
       sent to the real COM4 on your system) */

    p3io_uart_set_path(0, L"COM4");
    p3io_emu_init(&p3io_ddr_ops_with_plugs, NULL);
}

void p3io_ddr_fini(void)
{
    p3io_emu_fini();
}

static HRESULT p3io_ddr_read_jamma(void *ctx, uint32_t *state)
{
    log_assert(state != NULL);

    *state = ddr_io_read_pad();

    return S_OK;
}

static HRESULT p3io_ddr_set_outputs(void *ctx, uint32_t outputs)
{
    ddr_io_set_lights_p3io(outputs);

    return S_OK;
}

static HRESULT p3io_ddr_get_cab_type(void *ctx, enum p3io_cab_type *type)
{
    if (standard_def) {
        *type = P3IO_CAB_TYPE_SD;
    } else {
        *type = P3IO_CAB_TYPE_HD;
    }

    return S_OK;
}

static HRESULT p3io_ddr_get_video_freq(void *ctx, enum p3io_video_freq *freq)
{
    if (_15khz) {
        *freq = P3IO_VIDEO_FREQ_15KHZ;
    } else {
        *freq = P3IO_VIDEO_FREQ_31KHZ;
    }

    return S_OK;
}

static HRESULT p3io_ddr_get_roundplug(
    void *ctx, uint8_t plug_id, uint8_t *rom, uint8_t *eeprom)
{
    struct security_rp3_eeprom eeprom_out;

    if (plug_id == 0) {
        /* black */
        memcpy(rom, p3io_ddr_pcbid.id, sizeof(p3io_ddr_pcbid.id));
        security_rp3_generate_signed_eeprom_data(
            SECURITY_RP_UTIL_RP_TYPE_BLACK,
            &p3io_black_sign_key,
            &p3io_ddr_mcode,
            &p3io_ddr_pcbid,
            &eeprom_out);
    } else {
        /* white */
        memcpy(rom, p3io_ddr_eamid.id, sizeof(p3io_ddr_eamid.id));
        security_rp3_generate_signed_eeprom_data(
            SECURITY_RP_UTIL_RP_TYPE_WHITE,
            &p3io_white_sign_key,
            &security_mcode_eamuse,
            &p3io_ddr_eamid,
            &eeprom_out);
    }

    memcpy(eeprom, &eeprom_out, sizeof(struct security_rp3_eeprom));

    return S_OK;
}

// TODO coinstock
