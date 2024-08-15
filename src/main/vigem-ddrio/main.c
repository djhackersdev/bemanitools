#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "ViGEm/Client.h"

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"
#include "core/thread-crt.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface-io/ddr.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "util/math.h"

#include "vigemstub/helper.h"

#include "vigem-ddrio/config-vigem-ddrio.h"

#define NUM_PADS 2

static module_io_t *_module_io_ddr;

bool check_key(uint32_t input, size_t idx_in)
{
    if ((input >> idx_in) & 1) {
        return true;
    }

    return false;
}

uint32_t check_assign_key(uint32_t input, size_t idx_in, size_t bit_out)
{
    if (check_key(input, idx_in)) {
        return bit_out;
    }

    return 0;
}

uint32_t check_assign_light(uint32_t input, size_t idx_in, size_t gpio_out)
{
    if (check_key(input, idx_in)) {
        return 1 << gpio_out;
    }

    return 0;
}

void set_all_lights(bool on)
{
    bt_io_ddr_extio_lights_set(on ? UINT32_MAX : 0);
    bt_io_ddr_p3io_lights_set(on ? UINT32_MAX : 0);
    bt_io_ddr_hdxs_lights_panel_set(on ? UINT32_MAX : 0);

    for (int i = 0; i < 4; i++) {
        uint8_t val = on ? UINT8_MAX : 0;
        bt_io_ddr_hdxs_lights_rgb_set(i, val, val, val);
    }
}

void set_reactive_lights(uint32_t input_state)
{
    uint32_t new_extio_state = 0;
    uint32_t new_p3io_state = 0;
    uint32_t new_hdxs_state = 0;

    new_extio_state |= check_assign_light(
        input_state, BT_IO_DDR_P1_UP, BT_IO_DDR_EXTIO_LIGHT_P1_UP);
    new_extio_state |= check_assign_light(
        input_state, BT_IO_DDR_P1_DOWN, BT_IO_DDR_EXTIO_LIGHT_P1_DOWN);
    new_extio_state |= check_assign_light(
        input_state, BT_IO_DDR_P1_LEFT, BT_IO_DDR_EXTIO_LIGHT_P1_LEFT);
    new_extio_state |= check_assign_light(
        input_state, BT_IO_DDR_P1_RIGHT, BT_IO_DDR_EXTIO_LIGHT_P1_RIGHT);

    new_extio_state |= check_assign_light(
        input_state, BT_IO_DDR_P2_UP, BT_IO_DDR_EXTIO_LIGHT_P2_UP);
    new_extio_state |= check_assign_light(
        input_state, BT_IO_DDR_P2_DOWN, BT_IO_DDR_EXTIO_LIGHT_P2_DOWN);
    new_extio_state |= check_assign_light(
        input_state, BT_IO_DDR_P2_LEFT, BT_IO_DDR_EXTIO_LIGHT_P2_LEFT);
    new_extio_state |= check_assign_light(
        input_state, BT_IO_DDR_P2_RIGHT, BT_IO_DDR_EXTIO_LIGHT_P2_RIGHT);

    new_p3io_state |= check_assign_light(
        input_state, BT_IO_DDR_P1_START, BT_IO_DDR_P3IO_LIGHT_P1_MENU);
    new_p3io_state |= check_assign_light(
        input_state, BT_IO_DDR_P2_START, BT_IO_DDR_P3IO_LIGHT_P2_MENU);

    new_hdxs_state |= check_assign_light(
        input_state, BT_IO_DDR_P1_START, BT_IO_DDR_HDXS_LIGHT_HD_P1_START);
    new_hdxs_state |= check_assign_light(
        input_state, BT_IO_DDR_P1_MENU_UP, BT_IO_DDR_HDXS_LIGHT_HD_P1_UP_DOWN);
    new_hdxs_state |= check_assign_light(
        input_state,
        BT_IO_DDR_P1_MENU_DOWN,
        BT_IO_DDR_HDXS_LIGHT_HD_P1_UP_DOWN);
    new_hdxs_state |= check_assign_light(
        input_state,
        BT_IO_DDR_P1_MENU_LEFT,
        BT_IO_DDR_HDXS_LIGHT_HD_P1_LEFT_RIGHT);
    new_hdxs_state |= check_assign_light(
        input_state,
        BT_IO_DDR_P1_MENU_RIGHT,
        BT_IO_DDR_HDXS_LIGHT_HD_P1_LEFT_RIGHT);

    new_hdxs_state |= check_assign_light(
        input_state, BT_IO_DDR_P2_START, BT_IO_DDR_HDXS_LIGHT_HD_P2_START);
    new_hdxs_state |= check_assign_light(
        input_state, BT_IO_DDR_P2_MENU_UP, BT_IO_DDR_HDXS_LIGHT_HD_P2_UP_DOWN);
    new_hdxs_state |= check_assign_light(
        input_state,
        BT_IO_DDR_P2_MENU_DOWN,
        BT_IO_DDR_HDXS_LIGHT_HD_P2_UP_DOWN);
    new_hdxs_state |= check_assign_light(
        input_state,
        BT_IO_DDR_P2_MENU_LEFT,
        BT_IO_DDR_HDXS_LIGHT_HD_P2_LEFT_RIGHT);
    new_hdxs_state |= check_assign_light(
        input_state,
        BT_IO_DDR_P2_MENU_RIGHT,
        BT_IO_DDR_HDXS_LIGHT_HD_P2_LEFT_RIGHT);

    bt_io_ddr_extio_lights_set(new_extio_state);
    bt_io_ddr_p3io_lights_set(new_p3io_state);
    bt_io_ddr_hdxs_lights_panel_set(new_hdxs_state);
}

static void _module_io_ddr_init(module_io_t **module)
{
    bt_io_ddr_api_t api;

    module_io_ext_load_and_init(
        "ddrio.dll", "bt_module_io_ddr_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_ddr_api_set(&api);
}

int main(int argc, char **argv)
{
    core_log_bt_core_api_set();
    core_thread_crt_core_api_set();

    core_log_bt_ext_init_with_stdout();
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_INFO);

    struct vigem_ddrio_config config;

    if (!get_vigem_ddrio_config(&config)) {
        exit(EXIT_FAILURE);
    }

    _module_io_ddr_init(&_module_io_ddr);

    if (!bt_io_ddr_init()) {
        log_warning("Initializing ddrio failed");
        return -1;
    }

    // go ahead and turn lights off as some are on during boot.
    set_all_lights(false);

    PVIGEM_CLIENT client = vigem_helper_setup();

    if (!client) {
        log_warning("client failed to connect failed");
        return -1;
    }

    PVIGEM_TARGET pad[NUM_PADS];
    bool failed = false;

    for (uint8_t i = 0; i < NUM_PADS; i++) {
        pad[i] = vigem_helper_add_pad(client);

        if (!pad[i]) {
            log_warning("vigem_alloc pad %d failed", i);
            failed = true;
        }
    }

    if (failed) {
        bt_io_ddr_fini();
        bt_io_ddr_api_clear();
        module_io_free(&_module_io_ddr);
        return -1;
    }

    bool loop = true;
    uint32_t pad_state = 0;
    XUSB_REPORT state[NUM_PADS] = {0};

    log_info("vigem init succeeded, beginning poll loop");

    while (loop) {
        pad_state = bt_io_ddr_pad_read();

        for (uint8_t i = 0; i < NUM_PADS; i++) {
            memset(&state[i], 0, sizeof(state[i]));
        }

        state[0].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_TEST, XUSB_GAMEPAD_LEFT_THUMB);
        state[0].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_SERVICE, XUSB_GAMEPAD_RIGHT_THUMB);
        state[0].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_COIN, XUSB_GAMEPAD_BACK);

        // assign arrows to face buttons due to jumps / hat interpretation
        state[0].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P1_UP, XUSB_GAMEPAD_Y);
        state[0].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P1_DOWN, XUSB_GAMEPAD_A);
        state[0].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P1_LEFT, XUSB_GAMEPAD_X);
        state[0].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P1_RIGHT, XUSB_GAMEPAD_B);

        state[0].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P1_START, XUSB_GAMEPAD_START);
        state[0].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_P1_MENU_UP, XUSB_GAMEPAD_DPAD_UP);
        state[0].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_P1_MENU_DOWN, XUSB_GAMEPAD_DPAD_DOWN);
        state[0].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_P1_MENU_LEFT, XUSB_GAMEPAD_DPAD_LEFT);
        state[0].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_P1_MENU_RIGHT, XUSB_GAMEPAD_DPAD_RIGHT);

        state[1].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P2_UP, XUSB_GAMEPAD_Y);
        state[1].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P2_DOWN, XUSB_GAMEPAD_A);
        state[1].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P2_LEFT, XUSB_GAMEPAD_X);
        state[1].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P2_RIGHT, XUSB_GAMEPAD_B);

        state[1].wButtons |=
            check_assign_key(pad_state, BT_IO_DDR_P2_START, XUSB_GAMEPAD_START);
        state[1].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_P2_MENU_UP, XUSB_GAMEPAD_DPAD_UP);
        state[1].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_P2_MENU_DOWN, XUSB_GAMEPAD_DPAD_DOWN);
        state[1].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_P2_MENU_LEFT, XUSB_GAMEPAD_DPAD_LEFT);
        state[1].wButtons |= check_assign_key(
            pad_state, BT_IO_DDR_P2_MENU_RIGHT, XUSB_GAMEPAD_DPAD_RIGHT);

        for (uint8_t i = 0; i < NUM_PADS; i++) {
            vigem_target_x360_update(client, pad[i], state[i]);
        }

        if (config.enable_reactive_light) {
            set_reactive_lights(pad_state);
        }

        if (check_key(pad_state, BT_IO_DDR_TEST) &&
            check_key(pad_state, BT_IO_DDR_SERVICE)) {
            loop = false;
        }

        // avoid CPU banging
        Sleep(1);
    }

    for (uint8_t i = 0; i < NUM_PADS; i++) {
        vigem_target_remove(client, pad[i]);
        vigem_target_free(pad[i]);
    }

    vigem_free(client);

    bt_io_ddr_fini();
    bt_io_ddr_api_clear();
    module_io_free(&_module_io_ddr);

    return 0;
}
