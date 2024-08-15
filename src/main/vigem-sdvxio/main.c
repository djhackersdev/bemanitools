#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ViGEm/Client.h"

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/thread-crt.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/sdvx.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "util/math.h"

#include "vigemstub/helper.h"

#include "vigem-sdvxio/config-vigem-sdvxio.h"

#define ANALOG_FIXED_SENSITIVITY 1024

static module_io_t *_module_io_sdvx;

int16_t convert_analog_to_s16(uint16_t val)
{
    // val is 10 bit
    return (int64_t) (val * 64);
}

int16_t filter_floor(int32_t value, int16_t floor)
{
    if (abs(value) < floor) {
        return 0;
    }
    if (value > INT16_MAX) {
        value = INT16_MAX;
    }
    if (value < INT16_MIN) {
        value = INT16_MIN;
    }

    return value;
}

int32_t convert_relative_analog(
    uint16_t val, uint16_t last, int32_t buffered_last, int16_t multiplier)
{
    // val is 10 bit
    int16_t delta = get_wrapped_delta_s16(val, last, 1024);

    if (delta == 0) {
        // ease the stick back to 0 like a real stick would
        return buffered_last / 2.f;
    } else {
        int64_t result = buffered_last;
        result += delta * multiplier;

        // we use an i32 to store the buffered value
        // so that we can overshoot an i16 by up to 1.5x
        // this allows users to stay at the min/max stick positions
        // without perfect knob turning
        if (result > INT16_MAX * 1.5) {
            result = INT16_MAX * 1.5;
        }
        if (result < INT16_MIN * 1.5) {
            result = INT16_MIN * 1.5;
        }

        return result;
    }
}

bool check_key(uint16_t input, size_t idx_in)
{
    if ((input >> idx_in) & 1) {
        return true;
    }

    return false;
}

uint16_t check_assign_key(uint16_t input, size_t idx_in, size_t bit_out)
{
    if (check_key(input, idx_in)) {
        return bit_out;
    }

    return 0;
}

uint16_t check_assign_gpio(uint16_t input, size_t idx_in, size_t gpio_out)
{
    if (check_key(input, idx_in)) {
        return 1 << gpio_out;
    }

    return 0;
}

void gpio_keylight(uint16_t gpio0, uint16_t gpio1)
{
    uint16_t gpio_lights = 0;

    gpio_lights |= check_assign_gpio(
        gpio0, BT_IO_SDVX_IN_GPIO_0_START, BT_IO_SDVX_OUT_GPIO_START);
    gpio_lights |=
        check_assign_gpio(gpio0, BT_IO_SDVX_IN_GPIO_0_A, BT_IO_SDVX_OUT_GPIO_A);
    gpio_lights |=
        check_assign_gpio(gpio0, BT_IO_SDVX_IN_GPIO_0_B, BT_IO_SDVX_OUT_GPIO_B);
    gpio_lights |=
        check_assign_gpio(gpio0, BT_IO_SDVX_IN_GPIO_0_C, BT_IO_SDVX_OUT_GPIO_C);
    gpio_lights |=
        check_assign_gpio(gpio1, BT_IO_SDVX_IN_GPIO_1_D, BT_IO_SDVX_OUT_GPIO_D);
    gpio_lights |= check_assign_gpio(
        gpio1, BT_IO_SDVX_IN_GPIO_1_FX_L, BT_IO_SDVX_OUT_GPIO_FX_L);
    gpio_lights |= check_assign_gpio(
        gpio1, BT_IO_SDVX_IN_GPIO_1_FX_R, BT_IO_SDVX_OUT_GPIO_FX_R);

    bt_io_sdvx_gpio_lights_set(gpio_lights);
}

void set_pwm_brightness(uint8_t wing_pwm, uint8_t controller_pwm)
{
    // 0-11 are the 4 wings
    for (size_t i = 0; i < 12; ++i) {
        bt_io_sdvx_pwm_light_set(i, wing_pwm);
    }
    // 12-17 are the woofer / control deck
    for (size_t i = 12; i < 18; ++i) {
        bt_io_sdvx_pwm_light_set(i, controller_pwm);
    }
}

static void _io_sdvx_init(module_io_t **module)
{
    bt_io_sdvx_api_t api;

    module_io_ext_load_and_init(
        "sdvxio.dll", "bt_module_io_sdvx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_sdvx_api_set(&api);
}

int main(int argc, char **argv)
{
    core_thread_crt_core_api_set();
    core_log_bt_core_api_set();

    core_log_bt_ext_init_with_stdout();

    struct vigem_sdvxio_config config;
    if (!get_vigem_sdvxio_config(&config)) {
        exit(EXIT_FAILURE);
    }

    _io_sdvx_init(&_module_io_sdvx);

    if (!bt_io_sdvx_init()) {
        log_warning("Initializing sdvxio failed");
        return -1;
    }

    bt_io_sdvx_amp_volume_set(
        config.amp_volume, config.amp_volume, config.amp_volume);

    PVIGEM_CLIENT client = vigem_helper_setup();

    if (!client) {
        log_warning("client failed to connect failed");

        bt_io_sdvx_fini();

        bt_io_sdvx_api_clear();
        module_io_free(&_module_io_sdvx);

        return -1;
    }

    PVIGEM_TARGET pad = vigem_helper_add_pad(client);

    if (!pad) {
        log_warning("vigem_alloc pad 1 failed");

        bt_io_sdvx_fini();

        bt_io_sdvx_api_clear();
        module_io_free(&_module_io_sdvx);

        return -1;
    }

    bool loop = true;

    uint8_t sys;
    uint16_t gpio0;
    uint16_t gpio1;
    uint16_t vol[2] = {0, 0};
    uint16_t last_vol[2] = {0, 0};

    int32_t buffered_vol[2] = {0, 0};

    XUSB_REPORT state;

    log_info("vigem init succeeded, beginning poll loop");

    while (loop) {
        bt_io_sdvx_input_read();

        sys = bt_io_sdvx_input_gpio_sys_get();
        gpio0 = bt_io_sdvx_input_gpio_get(0);
        gpio1 = bt_io_sdvx_input_gpio_get(1);

        vol[0] = bt_io_sdvx_spinner_pos_get(0);
        vol[1] = bt_io_sdvx_spinner_pos_get(1);

        memset(&state, 0, sizeof(state));

        if (config.relative_analog) {
            buffered_vol[0] = convert_relative_analog(
                vol[0], last_vol[0], buffered_vol[0], ANALOG_FIXED_SENSITIVITY);
            buffered_vol[1] = convert_relative_analog(
                vol[1], last_vol[1], buffered_vol[1], ANALOG_FIXED_SENSITIVITY);

            state.sThumbLX =
                filter_floor(buffered_vol[0], ANALOG_FIXED_SENSITIVITY / 2);
            state.sThumbLY =
                filter_floor(buffered_vol[1], ANALOG_FIXED_SENSITIVITY / 2);

            last_vol[0] = vol[0];
            last_vol[1] = vol[1];
        } else {
            state.sThumbLX = convert_analog_to_s16(vol[0]);
            state.sThumbLY = convert_analog_to_s16(vol[1]);
        }

        state.wButtons |= check_assign_key(
            gpio0, BT_IO_SDVX_IN_GPIO_0_START, XUSB_GAMEPAD_START);
        state.wButtons |= check_assign_key(
            sys, BT_IO_SDVX_IN_GPIO_SYS_TEST, XUSB_GAMEPAD_BACK);
        state.wButtons |=
            check_assign_key(gpio0, BT_IO_SDVX_IN_GPIO_0_A, XUSB_GAMEPAD_A);
        state.wButtons |=
            check_assign_key(gpio0, BT_IO_SDVX_IN_GPIO_0_B, XUSB_GAMEPAD_B);
        state.wButtons |=
            check_assign_key(gpio0, BT_IO_SDVX_IN_GPIO_0_C, XUSB_GAMEPAD_X);
        state.wButtons |=
            check_assign_key(gpio1, BT_IO_SDVX_IN_GPIO_1_D, XUSB_GAMEPAD_Y);
        state.wButtons |= check_assign_key(
            gpio1, BT_IO_SDVX_IN_GPIO_1_FX_L, XUSB_GAMEPAD_LEFT_SHOULDER);
        state.wButtons |= check_assign_key(
            gpio1, BT_IO_SDVX_IN_GPIO_1_FX_R, XUSB_GAMEPAD_RIGHT_SHOULDER);

        vigem_target_x360_update(client, pad, state);

        if (config.enable_keylight) {
            gpio_keylight(gpio0, gpio1);
        }

        set_pwm_brightness(config.pwm_wings, config.pwm_controller);
        bt_io_sdvx_output_write();

        if (check_key(sys, BT_IO_SDVX_IN_GPIO_SYS_TEST) &&
            check_key(sys, BT_IO_SDVX_IN_GPIO_SYS_SERVICE)) {
            loop = false;
        }

        // avoid banging
        Sleep(1);
    }

    vigem_target_remove(client, pad);
    vigem_target_free(pad);

    vigem_free(client);

    bt_io_sdvx_amp_volume_set(96, 96, 96);
    Sleep(1000);

    bt_io_sdvx_fini();

    bt_io_sdvx_api_clear();
    module_io_free(&_module_io_sdvx);

    return 0;
}
