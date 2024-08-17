// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <dbt.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "api/core/log.h"
#include "api/core/thread.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface/input.h"

#include "module/input.h"

#include "main/module/input-ext.h"
#include "main/module/input.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/input.h"
#include "sdk/module/io/eam.h"

#include "eamio/eam-config.h"
#include "eamio/eam-impl.h"
#include "eamio/eam-s11n.h"

#include "util/fs.h"
#include "util/msg-thread.h"

static void eam_handle_hotplug_msg(WPARAM wparam, const DEV_BROADCAST_HDR *hdr);
static FILE *eam_io_config_open(const char *mode);
static void eam_io_config_load(void);
static void eam_io_config_save(void);
static bool eam_io_get_autogen(void);
static void eam_io_set_autogen(bool autogen);
static bool eam_io_get_alt_10k(void);
static void eam_io_set_alt_10k(bool alt_10k);
static struct hid_stub *eam_io_get_keypad_device(uint8_t unit_no);
static void eam_io_set_keypad_device(uint8_t unit_no, struct hid_stub *hid);
static const char *eam_io_get_card_path(uint8_t unit_no);
static void eam_io_set_card_path(uint8_t unit_no, const char *path);

static module_input_t *_eam_io_module_input;
static HANDLE eam_hinst;
static struct eam *eam_inst;

static const struct bt_io_eam_config_api eam_io_config_api = {
    .config_save = eam_io_config_save,
    .get_autogen = eam_io_get_autogen,
    .set_autogen = eam_io_set_autogen,
    .get_alt_10k = eam_io_get_alt_10k,
    .set_alt_10k = eam_io_set_alt_10k,
    .get_keypad_device = eam_io_get_keypad_device,
    .set_keypad_device = eam_io_set_keypad_device,
    .get_card_path = eam_io_get_card_path,
    .set_card_path = eam_io_set_card_path,
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
    return fopen_appdata("DJHACKERS", "eam_v4_22.bin", mode);
}

bool bt_io_eam_init()
{
    bool result;
    bt_input_api_t input_api;

    module_input_ext_load_and_init("geninput.dll", &_eam_io_module_input);
    module_input_api_get(_eam_io_module_input, &input_api);
    bt_input_api_set(&input_api);

    result = bt_input_init();

    if (!result) {
        return false;
    }

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

void bt_io_eam_fini(void)
{
    msg_thread_fini();
    eam_impl_destroy(eam_inst);

    bt_input_api_clear();
    module_input_free(&_eam_io_module_input);
}

const bt_io_eam_config_api_t *bt_io_eam_config_api_get(void)
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

static bool eam_io_get_autogen(void)
{
    return eam_impl_get_autogen(eam_inst);
}

static void eam_io_set_autogen(bool autogen)
{
    eam_impl_set_autogen(eam_inst, autogen);
}

static bool eam_io_get_alt_10k(void)
{
    return eam_impl_get_alt_10k(eam_inst);
}

static void eam_io_set_alt_10k(bool alt_10k)
{
    eam_impl_set_alt_10k(eam_inst, alt_10k);
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

uint16_t bt_io_eam_keypad_state_get(uint8_t unit_no)
{
    return eam_impl_get_keypad_state(eam_inst, unit_no);
}

uint8_t bt_io_eam_sensor_state_get(uint8_t unit_no)
{
    if (eam_impl_get_sensor_state(eam_inst, unit_no)) {
        return (1 << BT_IO_EAM_SENSOR_STATE_FRONT) |
            (1 << BT_IO_EAM_SENSOR_STATE_BACK);
    } else {
        return 0x00;
    }
}

uint8_t bt_io_eam_card_read(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    return eam_impl_read_card(eam_inst, unit_no, card_id, nbytes);
}

bool bt_io_eam_card_slot_cmd_send(uint8_t unit_no, uint8_t cmd)
{
    // ignored
    return true;
}

bool bt_io_eam_poll(uint8_t unit_no)
{
    // ignored
    return true;
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_eam_api_get(bt_io_eam_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_eam_init;
    api->v1.fini = bt_io_eam_fini;
    api->v1.keypad_state_get = bt_io_eam_keypad_state_get;
    api->v1.sensor_state_get = bt_io_eam_sensor_state_get;
    api->v1.card_read = bt_io_eam_card_read;
    api->v1.card_slot_cmd_send = bt_io_eam_card_slot_cmd_send;
    api->v1.poll = bt_io_eam_poll;
    api->v1.config_api_get = bt_io_eam_config_api_get;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        eam_hinst = hinst;
    }

    return TRUE;
}
