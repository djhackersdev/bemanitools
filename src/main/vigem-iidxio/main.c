#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <ViGEm/Client.h>

#include "bemanitools/iidxio.h"

#include "util/log.h"
#include "util/math.h"
#include "util/thread.h"
#include "util/time.h"

#include "vigem-iidxio/cab-16seg-sequencer.h"
#include "vigem-iidxio/cab-light-sequencer.h"
#include "vigem-iidxio/config.h"

#include "vigemstub/helper.h"

#define ANALOG_FIXED_SENSITIVITY 256

static const uint8_t JOYSTICKS_NUM = 3;

static int16_t _convert_analog_to_s16(uint8_t val)
{
    return (int64_t) val * 256;
}

static int16_t _filter_floor(int32_t value, int16_t floor) {
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

static int32_t _convert_relative_analog(
    uint8_t val, uint8_t last, int32_t buffered_last, int16_t multiplier)
{
    int16_t delta = get_wrapped_delta_s16(val, last, UINT8_MAX);

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

static bool _check_key(uint16_t input, size_t idx_in)
{
    if ((input >> idx_in) & 1) {
        return true;
    }

    return false;
}

static uint16_t _check_assign_key(uint16_t input, size_t idx_in, size_t bit_out)
{
    if (_check_key(input, idx_in)) {
        return bit_out;
    }

    return 0;
}

int main(int argc, char **argv)
{
    log_to_writer(log_writer_stdout, NULL);

    struct vigem_iidxio_config config;

    if (!vigem_iidxio_config_get(&config)) {
        return -1;
    }

    iidx_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!iidx_io_init(crt_thread_create, crt_thread_join, crt_thread_destroy)) {
        log_warning("Initializing iidxio failed");
        return -1;
    }

    PVIGEM_CLIENT client = vigem_helper_setup();

    if (!client) {
        log_warning("VIGEM client failed to connect");

        iidx_io_fini();

        return -1;
    }

    PVIGEM_TARGET pad[JOYSTICKS_NUM];
    bool failed;

    failed = false;
    
    for (uint8_t i = 0; i < JOYSTICKS_NUM; i++) {
        pad[i] = vigem_helper_add_pad(client);

        if (!pad[i]) {
            log_warning("vigem_alloc pad %d failed", i);
            failed = true;
        }
    }

    if (failed) {
        iidx_io_fini();
        return -1;
    }

    bool loop = true;

    XUSB_REPORT state[JOYSTICKS_NUM];

    log_info("vigem init succeeded, beginning poll loop");

    // Initial output state, turn all lights off
    iidx_io_ep1_set_deck_lights(0);
    iidx_io_ep1_set_panel_lights(0);
    iidx_io_ep1_set_top_lamps(0);
    iidx_io_ep1_set_top_neons(false);

    vigem_iidxio_cab_light_sequencer_init(config.cab_light_mode);

    if (config.text_16seg[0] != '\0') {
        vigem_iidxio_cab_16seg_sequencer_init(config.text_16seg, config.text_scroll_cycle_time_ms);
    }

    uint8_t turntable_buffered[2] = {0, 0};
    uint8_t turntable_last[2] = {0, 0};

    while (loop) {
        if (!iidx_io_ep2_recv()) {
            log_warning("iidxio receiving failed");
            break;
        }

        for (uint8_t i = 0; i < JOYSTICKS_NUM; i++) {
            memset(&state[i], 0, sizeof(state[i]));
        }
        
        // 14 keys
        uint16_t keys = iidx_io_ep2_get_keys();

        state[0].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P1_1, XUSB_GAMEPAD_A);
        state[0].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P1_2, XUSB_GAMEPAD_B);
        state[0].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P1_3, XUSB_GAMEPAD_X);
        state[0].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P1_4, XUSB_GAMEPAD_Y);
        state[0].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P1_5, XUSB_GAMEPAD_LEFT_SHOULDER);
        state[0].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P1_6, XUSB_GAMEPAD_RIGHT_SHOULDER);
        state[0].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P1_7, XUSB_GAMEPAD_BACK);

        state[1].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P2_1, XUSB_GAMEPAD_A);
        state[1].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P2_2, XUSB_GAMEPAD_B);
        state[1].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P2_3, XUSB_GAMEPAD_X);
        state[1].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P2_4, XUSB_GAMEPAD_Y);
        state[1].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P2_5, XUSB_GAMEPAD_LEFT_SHOULDER);
        state[1].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P2_6, XUSB_GAMEPAD_RIGHT_SHOULDER);
        state[1].wButtons |= _check_assign_key(
                keys, IIDX_IO_KEY_P2_7, XUSB_GAMEPAD_BACK);

        // Panel buttons
        uint8_t panel = iidx_io_ep2_get_panel();

        state[0].wButtons |= _check_assign_key(
                panel, IIDX_IO_PANEL_LIGHT_P1_START, XUSB_GAMEPAD_START);
        state[1].wButtons |= _check_assign_key(
                panel, IIDX_IO_PANEL_LIGHT_P2_START, XUSB_GAMEPAD_START);

        state[2].wButtons |= _check_assign_key(
                panel, IIDX_IO_PANEL_LIGHT_VEFX, XUSB_GAMEPAD_B);
        state[2].wButtons |= _check_assign_key(
                panel, IIDX_IO_PANEL_LIGHT_EFFECT, XUSB_GAMEPAD_A);

        // System buttons
        uint8_t system = iidx_io_ep2_get_sys();

        state[2].wButtons |= _check_assign_key(
                system, IIDX_IO_SYS_TEST, XUSB_GAMEPAD_X);
        state[2].wButtons |= _check_assign_key(
                system, IIDX_IO_SYS_SERVICE, XUSB_GAMEPAD_Y);
        state[2].wButtons |= _check_assign_key(
                system, IIDX_IO_SYS_COIN, XUSB_GAMEPAD_START);

        // Turntable
        uint8_t turntable[2];
        
        turntable[0] = iidx_io_ep2_get_turntable(0);
        turntable[1] = iidx_io_ep2_get_turntable(1);

        if (config.relative_analog) {
            turntable_buffered[0] = _convert_relative_analog(
                turntable[0], turntable_last[0], turntable_buffered[0], ANALOG_FIXED_SENSITIVITY);
            turntable_buffered[1] = _convert_relative_analog(
                turntable[1], turntable_last[1], turntable_buffered[1], ANALOG_FIXED_SENSITIVITY);

            state[0].sThumbLX = _filter_floor(turntable_buffered[0], ANALOG_FIXED_SENSITIVITY / 2);
            state[1].sThumbLX = _filter_floor(turntable_buffered[1], ANALOG_FIXED_SENSITIVITY / 2);

            turntable_last[0] = turntable[0];
            turntable_last[1] = turntable[1];
        } else {
            state[0].sThumbLX = _convert_analog_to_s16(turntable[0]);
            state[1].sThumbLX = _convert_analog_to_s16(turntable[1]);
        }

        vigem_target_x360_update(client, pad[0], state[0]);
        vigem_target_x360_update(client, pad[1], state[1]);

        // -----------------------------------------------------------------------------
        
        // Light related outputs

        if (config.enable_keylight) {
            iidx_io_ep1_set_deck_lights(keys);
            iidx_io_ep1_set_panel_lights(panel);
        }

        if (config.cab_light_mode != LIGHT_SEQ_MODE_OFF) {
            bool neon;
            uint8_t spots;

            neon = false;
            spots = 0;

            vigem_iidxio_cab_light_sequencer_update(
                keys,
                turntable[0],
                turntable[1],
                &neon,
                &spots);

            iidx_io_ep1_set_top_neons(neon);
            iidx_io_ep1_set_top_lamps(spots);
        }

        char buffer_16seg[9];

        vigem_iidxio_cab_16seg_sequencer_update(buffer_16seg);
        iidx_io_ep3_write_16seg(buffer_16seg);

        if (!iidx_io_ep1_send()) {
            log_warning("iidxio sending failed");
            break;
        }
        
        if (_check_key(system, IIDX_IO_SYS_TEST) &&
                _check_key(system, IIDX_IO_SYS_SERVICE)) {
            log_info("Test + service pressed, exiting...");
            loop = false;
        }

        // avoid banging
        Sleep(1);
    }

    // Cleanup, turn all lights off
    iidx_io_ep1_set_deck_lights(0);
    iidx_io_ep1_set_panel_lights(0);
    iidx_io_ep1_set_top_lamps(0);
    iidx_io_ep1_set_top_neons(false);

    iidx_io_ep1_send();
    // Required to handle iidxio-ezusb specific quirks with flushing 16seg text
    iidx_io_ep2_recv();
    iidx_io_ep3_write_16seg("         ");

    Sleep(10);

    for (uint8_t i = 0; i < JOYSTICKS_NUM; i++) {
        vigem_target_remove(client, pad[i]);
        vigem_target_free(pad[i]);
    }

    vigem_free(client);

    Sleep(1000);

    iidx_io_fini();

    return 0;
}
