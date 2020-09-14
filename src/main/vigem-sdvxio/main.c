#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <Xinput.h>
#include <windows.h>

#include "ViGEm/Client.h"

#include "bemanitools/sdvxio.h"
#include "util/log.h"
#include "util/thread.h"
#include "vigemstub/helper.h"

#include "vigem-sdvxio/config-vigem-sdvxio.h"

int64_t convert_analog_to_s16(uint16_t val)
{
    // val is 10 bit
    return (int64_t)(val * 64);
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
    get_vigem_sdvxio_config(&config);

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

        state.sThumbLX = convert_analog_to_s16(vol[0]);
        state.sThumbRX = convert_analog_to_s16(vol[1]);

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

    sdvx_io_set_amp_volume(96, 96, 96);
    Sleep(1000);

    sdvx_io_fini();

    return 0;
}
