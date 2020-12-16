#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "ViGEm/Client.h"

#include "bemanitools/sdvxio.h"
#include "util/log.h"
#include "util/math.h"
#include "util/thread.h"
#include "vigemstub/helper.h"

#include "vigem-sdvxio/config-vigem-sdvxio.h"

#define ANALOG_FIXED_SENSITIVITY 1024

int16_t convert_analog_to_s16(uint16_t val)
{
    // val is 10 bit
    return (int64_t)(val * 64);
}

int16_t filter_floor(int32_t value, int16_t floor) {
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
        if (result > INT16_MAX*1.5) {
            result = INT16_MAX*1.5;
        }
        if (result < INT16_MIN*1.5) {
            result = INT16_MIN*1.5;
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
        gpio0, SDVX_IO_IN_GPIO_0_START, SDVX_IO_OUT_GPIO_START);
    gpio_lights |=
        check_assign_gpio(gpio0, SDVX_IO_IN_GPIO_0_A, SDVX_IO_OUT_GPIO_A);
    gpio_lights |=
        check_assign_gpio(gpio0, SDVX_IO_IN_GPIO_0_B, SDVX_IO_OUT_GPIO_B);
    gpio_lights |=
        check_assign_gpio(gpio0, SDVX_IO_IN_GPIO_0_C, SDVX_IO_OUT_GPIO_C);
    gpio_lights |=
        check_assign_gpio(gpio1, SDVX_IO_IN_GPIO_1_D, SDVX_IO_OUT_GPIO_D);
    gpio_lights |=
        check_assign_gpio(gpio1, SDVX_IO_IN_GPIO_1_FX_L, SDVX_IO_OUT_GPIO_FX_L);
    gpio_lights |=
        check_assign_gpio(gpio1, SDVX_IO_IN_GPIO_1_FX_R, SDVX_IO_OUT_GPIO_FX_R);

    sdvx_io_set_gpio_lights(gpio_lights);
}

void set_pwm_brightness(uint8_t wing_pwm, uint8_t controller_pwm)
{
    // 0-11 are the 4 wings
    for (size_t i = 0; i < 12; ++i) {
        sdvx_io_set_pwm_light(i, wing_pwm);
    }
    // 12-17 are the woofer / control deck
    for (size_t i = 12; i < 18; ++i) {
        sdvx_io_set_pwm_light(i, controller_pwm);
    }
}

int main(int argc, char **argv)
{
    log_to_writer(log_writer_stdout, NULL);

    struct vigem_sdvxio_config config;
    if (!get_vigem_sdvxio_config(&config)) {
        exit(EXIT_FAILURE);
    }

    sdvx_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!sdvx_io_init(crt_thread_create, crt_thread_join, crt_thread_destroy)) {
        log_warning("Initializing sdvxio failed");
        return -1;
    }

    sdvx_io_set_amp_volume(config.amp_volume, config.amp_volume, config.amp_volume);

    PVIGEM_CLIENT client = vigem_helper_setup();

    if (!client) {
        log_warning("client failed to connect failed");
        return -1;
    }

    PVIGEM_TARGET pad = vigem_helper_add_pad(client);

    if (!pad) {
        log_warning("vigem_alloc pad 1 failed");
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
        sdvx_io_read_input();

        sys = sdvx_io_get_input_gpio_sys();
        gpio0 = sdvx_io_get_input_gpio(0);
        gpio1 = sdvx_io_get_input_gpio(1);

        vol[0] = sdvx_io_get_spinner_pos(0);
        vol[1] = sdvx_io_get_spinner_pos(1);

        memset(&state, 0, sizeof(state));

        if (config.relative_analog) {
            buffered_vol[0] = convert_relative_analog(vol[0], last_vol[0], buffered_vol[0], ANALOG_FIXED_SENSITIVITY);
            buffered_vol[1] = convert_relative_analog(vol[1], last_vol[1], buffered_vol[1], ANALOG_FIXED_SENSITIVITY);

            state.sThumbLX = filter_floor(buffered_vol[0], ANALOG_FIXED_SENSITIVITY/2);
            state.sThumbLY = filter_floor(buffered_vol[1], ANALOG_FIXED_SENSITIVITY/2);

            last_vol[0] = vol[0];
            last_vol[1] = vol[1];
        } else {
            state.sThumbLX = convert_analog_to_s16(vol[0]);
            state.sThumbLY = convert_analog_to_s16(vol[1]);
        }

        state.wButtons |= check_assign_key(
            gpio0, SDVX_IO_IN_GPIO_0_START, XUSB_GAMEPAD_START);
        state.wButtons |=
            check_assign_key(sys, SDVX_IO_IN_GPIO_SYS_TEST, XUSB_GAMEPAD_BACK);
        state.wButtons |=
            check_assign_key(gpio0, SDVX_IO_IN_GPIO_0_A, XUSB_GAMEPAD_A);
        state.wButtons |=
            check_assign_key(gpio0, SDVX_IO_IN_GPIO_0_B, XUSB_GAMEPAD_B);
        state.wButtons |=
            check_assign_key(gpio0, SDVX_IO_IN_GPIO_0_C, XUSB_GAMEPAD_X);
        state.wButtons |=
            check_assign_key(gpio1, SDVX_IO_IN_GPIO_1_D, XUSB_GAMEPAD_Y);
        state.wButtons |= check_assign_key(
            gpio1, SDVX_IO_IN_GPIO_1_FX_L, XUSB_GAMEPAD_LEFT_SHOULDER);
        state.wButtons |= check_assign_key(
            gpio1, SDVX_IO_IN_GPIO_1_FX_R, XUSB_GAMEPAD_RIGHT_SHOULDER);

        vigem_target_x360_update(client, pad, state);

        if (config.enable_keylight) {
            gpio_keylight(gpio0, gpio1);
        }

        set_pwm_brightness(config.pwm_wings, config.pwm_controller);
        sdvx_io_write_output();

        if (check_key(sys, SDVX_IO_IN_GPIO_SYS_TEST) &&
            check_key(sys, SDVX_IO_IN_GPIO_SYS_SERVICE)) {
            loop = false;
        }

        // avoid banging
        Sleep(1);
    }

    vigem_target_remove(client, pad);
    vigem_target_free(pad);

    vigem_free(client);

    sdvx_io_set_amp_volume(96, 96, 96);
    Sleep(1000);

    sdvx_io_fini();

    return 0;
}
