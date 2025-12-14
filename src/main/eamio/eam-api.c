// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <dbt.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bemanitools/eamio.h"
#include "bemanitools/input.h"

#include "eamio/eam-config.h"
#include "eamio/eam-impl.h"
#include "eamio/eam-s11n.h"
#include "eamio/eam-scard.h"

#include "util/fs.h"
#include "util/log.h"
#include "util/msg-thread.h"
#include "util/thread.h"

static void eam_handle_hotplug_msg(WPARAM wparam, const DEV_BROADCAST_HDR *hdr);
static FILE *eam_io_config_open(const char *mode);
static void eam_io_config_load(void);
static void eam_io_config_save(void);
static bool eam_io_get_scard(void);
static void eam_io_set_scard(bool scard);
static bool eam_io_get_mux_keypad(void);
static void eam_io_set_mux_keypad(bool mux_keypad);
static bool eam_io_get_mux_scard(void);
static void eam_io_set_mux_scard(bool mux_scard);
static bool eam_io_get_autogen(void);
static void eam_io_set_autogen(bool autogen);
static bool eam_io_do_keypads_conflict(void);
static bool eam_io_get_alt_10k(uint8_t unit_no);
static void eam_io_set_alt_10k(uint8_t unit_no, bool alt_10k);
static struct hid_stub *eam_io_get_keypad_device(uint8_t unit_no);
static void eam_io_set_keypad_device(uint8_t unit_no, struct hid_stub *hid);
static const char *eam_io_get_card_path(uint8_t unit_no);
static void eam_io_set_card_path(uint8_t unit_no, const char *path);

static HANDLE eam_hinst;
static struct eam *eam_inst;

static const struct eam_io_config_api eam_io_config_api = {
    .config_save = eam_io_config_save,
    .get_autogen = eam_io_get_autogen,
    .set_autogen = eam_io_set_autogen,
    .get_alt_10k = eam_io_get_alt_10k,
    .set_alt_10k = eam_io_set_alt_10k,
    .get_keypad_device = eam_io_get_keypad_device,
    .set_keypad_device = eam_io_set_keypad_device,
    .get_card_path = eam_io_get_card_path,
    .set_card_path = eam_io_set_card_path,
    .get_scard = eam_io_get_scard,
    .set_scard = eam_io_set_scard,
    .get_mux_keypad = eam_io_get_mux_keypad,
    .set_mux_keypad = eam_io_set_mux_keypad,
    .get_mux_scard = eam_io_get_mux_scard,
    .set_mux_scard = eam_io_set_mux_scard,
    .do_keypads_conflict = eam_io_do_keypads_conflict,
    .get_scard_reader_name = eam_scard_get_reader_name,
};

void msg_window_setup(HWND hwnd)
{
    log_info(
        "Drive insertion listener ready, thread id = %d",
        (int) GetCurrentThreadId());
}

LRESULT WINAPI
msg_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_DEVICECHANGE:
            eam_handle_hotplug_msg(wparam, (DEV_BROADCAST_HDR *) lparam);

            return TRUE;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

static void eam_handle_hotplug_msg(WPARAM wparam, const DEV_BROADCAST_HDR *hdr)
{
    uint32_t drives;
    unsigned long bit;
    const DEV_BROADCAST_VOLUME *vol;

    if (wparam != DBT_DEVICEARRIVAL) {
        return;
    }

    if (hdr->dbch_devicetype != DBT_DEVTYP_VOLUME) {
        return;
    }

    vol = (const DEV_BROADCAST_VOLUME *) hdr;
    drives = vol->dbcv_unitmask;

    while (_BitScanForward(&bit, drives)) {
        drives &= ~(1 << bit);
        eam_impl_notify_hotplug(eam_inst, (uint8_t) bit);
    }
}

void msg_window_teardown(HWND hwnd)
{
    log_info("Volume insertion listener shutting down");
}

static FILE *eam_io_config_open(const char *mode)
{
    return fopen_appdata("DJHACKERS", "eam_v5_49a.bin", mode);
}

void eam_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    log_to_external(misc, info, warning, fatal);
}

bool eam_io_init(
    thread_create_t create, thread_join_t join, thread_destroy_t destroy)
{
    input_init(create, join, destroy);
    thread_api_init(create, join, destroy);
    eam_io_config_load();
    msg_thread_init(eam_hinst);

    return true;
}

static void eam_io_config_load(void)
{
    struct eam *eam;
    FILE *f;

    f = eam_io_config_open("rb");

    if (f == NULL) {
        log_warning("Failed to open eam config, using defaults");

        goto open_fail;
    }

    eam = eam_impl_config_load(f);

    if (eam == NULL) {
        log_warning("eam config is corrupted, using defaults");

        goto read_fail;
    }

    eam_inst = eam;

    fclose(f);

    return;

read_fail:
    fclose(f);

open_fail:
    eam_inst = eam_impl_create();
}

void eam_io_fini(void)
{
    msg_thread_fini();
    eam_impl_destroy(eam_inst);
    input_fini();
    eam_scard_fini();
}

const struct eam_io_config_api *eam_io_get_config_api(void)
{
    return &eam_io_config_api;
}

static void eam_io_config_save(void)
{
    FILE *f;

    f = eam_io_config_open("wb");

    if (f == NULL) {
        return;
    }

    eam_impl_config_save(eam_inst, f);
    fclose(f);
}

static bool eam_io_get_scard(void)
{
    return eam_impl_get_scard(eam_inst);
}

static void eam_io_set_scard(bool scard)
{
    eam_impl_set_scard(eam_inst, scard);
}

static bool eam_io_get_mux_keypad(void)
{
    return eam_impl_get_mux_keypad(eam_inst);
}

static void eam_io_set_mux_keypad(bool mux_keypad)
{
    eam_impl_set_mux_keypad(eam_inst, mux_keypad);
}

static bool eam_io_get_mux_scard(void)
{
    return eam_impl_get_mux_scard(eam_inst);
}

static void eam_io_set_mux_scard(bool mux_scard)
{
    eam_impl_set_mux_scard(eam_inst, mux_scard);
}

static bool eam_io_get_autogen(void)
{
    return eam_impl_get_autogen(eam_inst);
}

static void eam_io_set_autogen(bool autogen)
{
    eam_impl_set_autogen(eam_inst, autogen);
}

static bool eam_io_do_keypads_conflict(void)
{
    return eam_impl_do_keypads_conflict(eam_inst);
}

static bool eam_io_get_alt_10k(uint8_t unit_no)
{
    return eam_impl_get_alt_10k(eam_inst, unit_no);
}

static void eam_io_set_alt_10k(uint8_t unit_no, bool alt_10k)
{
    eam_impl_set_alt_10k(eam_inst, unit_no, alt_10k);
}

static struct hid_stub *eam_io_get_keypad_device(uint8_t unit_no)
{
    return eam_impl_get_keypad_device(eam_inst, unit_no);
}

static void eam_io_set_keypad_device(uint8_t unit_no, struct hid_stub *hid)
{
    eam_impl_set_keypad_device(eam_inst, unit_no, hid);
}

static const char *eam_io_get_card_path(uint8_t unit_no)
{
    return eam_impl_get_card_path(eam_inst, unit_no);
}

static void eam_io_set_card_path(uint8_t unit_no, const char *path)
{
    eam_impl_set_card_path(eam_inst, unit_no, path);
}

uint16_t eam_io_get_keypad_state(uint8_t unit_no)
{
    return eam_impl_get_keypad_state(eam_inst, unit_no);
}

uint8_t eam_io_get_sensor_state(uint8_t unit_no)
{
    bool has_card;

    if (eam_io_get_scard()) {
        has_card = eam_scard_is_present(unit_no);
    } else {
        has_card = eam_impl_get_sensor_state(eam_inst, unit_no);
    }

    if (has_card) {
        return (1 << EAM_IO_SENSOR_FRONT) | (1 << EAM_IO_SENSOR_BACK);
    } else {
        return 0x00;
    }    
}

uint8_t eam_io_read_card(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    if (eam_io_get_scard()) {
        return eam_scard_read(unit_no, card_id, nbytes);
    } else {
        return eam_impl_read_card(eam_inst, unit_no, card_id, nbytes);
    }
}

bool eam_io_card_slot_cmd(uint8_t unit_no, uint8_t cmd)
{
    // ignored
    return true;
}

bool eam_io_poll(uint8_t unit_no)
{
    if (eam_io_get_scard()) {
        eam_scard_poll(unit_no);
    }
	
    return true;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, void *scard)
{
    if (reason == DLL_PROCESS_ATTACH) {
        eam_hinst = hinst;
    }

    return TRUE;
}
