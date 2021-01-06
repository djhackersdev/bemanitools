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

#define JOYSTICKS_NUM 3
#define TURNTABLE_NUM 2

static uint8_t _tt_last_raw[TURNTABLE_NUM];
static int32_t _tt_state_for_analog[TURNTABLE_NUM];
static int16_t _tt_state_for_btn[TURNTABLE_NUM];

static int16_t _convert_analog_to_s16(uint8_t val)
{
    // TT analog value is 8 bit, upscale to 16 bit
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

static int32_t _handle_turntable_analog(
        const struct vigem_iidxio_config* config,
        uint8_t tt_cur,
        uint8_t tt_last,
        int32_t tt_state,
        XUSB_REPORT *pad_state)
{
    int32_t state = tt_state;

    if (config->tt.analog.relative) {
        state = _convert_relative_analog(
            tt_cur, tt_last, state, config->tt.analog.relative_sensitivity);

        pad_state->sThumbLX = _filter_floor(state, config->tt.analog.relative_sensitivity / 2);
    } else {
        pad_state->sThumbLX = _convert_analog_to_s16(tt_cur);
    }

    return state;
}

static int16_t _handle_turntable_as_button(
        const struct vigem_iidxio_config* config, 
        uint8_t tt_cur,
        uint8_t tt_last,
        int16_t tt_state,
        XUSB_REPORT *pad_state)
{
    const uint8_t btn_inc = config->tt.button.debounce;
    const uint16_t btn_threshhold = btn_inc * config->tt.button.threshold;
    const uint16_t btn_max = btn_threshhold * 2;

    int8_t delta = tt_cur - tt_last;
    int16_t state = tt_state;

    if (config->tt.debug_output) {
        printf("delta_button %d, ", delta);
    }

    // TT direction changed suddendly -> fast scratchin' action like a real DJ
    // Reset state to avoid lag with state having to decrease from high values
    if (delta > 0 && delta >= config->tt.button.threshold && state < 0) {
        state = 0;
    } else if (delta < 0 && abs(delta) >= config->tt.button.threshold && state > 0) {
        state = 0;
    }

    // Update state according to delta, debounce and max values
    if (delta > 0 && state < btn_max) {
        state += btn_inc;
    } else if (delta < 0 && abs(state) < btn_max) {
        state -= btn_inc;
    }

    // With each update, automatically counter the player's TT movement to turn the state
    // back to 0, e.g. player moves TT -> state += 20. with no additional movement, neutral state
    // (0) is back in 20 ticks
    if (state > 0) {
        state--;
    } else if (state < 0) {
        state++;
    }

    if (state > btn_threshhold) {
        pad_state->wButtons |= XUSB_GAMEPAD_LEFT_THUMB;
    } else if (state < (int16_t) btn_threshhold * -1) {
        pad_state->wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
    }

    return state;
}

static void _handle_turntable(
        const struct vigem_iidxio_config* config,
        uint8_t* tt,
        XUSB_REPORT *state)
{
    for (uint8_t i = 0; i < TURNTABLE_NUM; i++) {
        if (config->tt.debug_output) {
            printf("TT (%d): last_raw %d, raw %d, ", i, _tt_last_raw[i], tt[i]);
        }

        _tt_state_for_btn[i] = _handle_turntable_as_button(
            config, tt[i], _tt_last_raw[i], _tt_state_for_btn[i], &state[i]);

        _tt_state_for_analog[i] = _handle_turntable_analog(
            config, tt[i], _tt_last_raw[i], _tt_state_for_analog[i], &state[i]);

        _tt_last_raw[i] = tt[i];

        if (config->tt.debug_output) {
            printf("state_btn %d, state_rel_analog %d\n", 
                _tt_state_for_btn[i],  _tt_state_for_analog[i]);
        }
    }
}

static void _handle_buttons_14keys(uint16_t keys, XUSB_REPORT *state)
{
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
}

static void _handle_buttons_panel(uint8_t panel, XUSB_REPORT *state)
{
    state[0].wButtons |= _check_assign_key(
            panel, IIDX_IO_PANEL_LIGHT_P1_START, XUSB_GAMEPAD_START);
    state[1].wButtons |= _check_assign_key(
            panel, IIDX_IO_PANEL_LIGHT_P2_START, XUSB_GAMEPAD_START);

    state[2].wButtons |= _check_assign_key(
            panel, IIDX_IO_PANEL_LIGHT_VEFX, XUSB_GAMEPAD_B);
    state[2].wButtons |= _check_assign_key(
            panel, IIDX_IO_PANEL_LIGHT_EFFECT, XUSB_GAMEPAD_A);
}

static void _handle_buttons_system(uint8_t system, XUSB_REPORT *state)
{
    state[2].wButtons |= _check_assign_key(
            system, IIDX_IO_SYS_TEST, XUSB_GAMEPAD_X);
    state[2].wButtons |= _check_assign_key(
            system, IIDX_IO_SYS_SERVICE, XUSB_GAMEPAD_Y);
    state[2].wButtons |= _check_assign_key(
            system, IIDX_IO_SYS_COIN, XUSB_GAMEPAD_START);
}

static void _all_lights_off()
{
    iidx_io_ep1_set_deck_lights(0);
    iidx_io_ep1_set_panel_lights(0);
    iidx_io_ep1_set_top_lamps(0);
    iidx_io_ep1_set_top_neons(false);
    iidx_io_ep3_write_16seg("         ");
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

    // Create clean output state
    _all_lights_off();

    vigem_iidxio_cab_light_sequencer_init(config.cab_light.light_mode);

    if (config.cab_light.text_16seg[0] != '\0') {
        vigem_iidxio_cab_16seg_sequencer_init(
            config.cab_light.text_16seg, config.cab_light.text_scroll_cycle_time_ms);
    }

    while (loop) {
        for (uint8_t i = 0; i < JOYSTICKS_NUM; i++) {
            memset(&state[i], 0, sizeof(state[i]));
        }
        
        if (!iidx_io_ep2_recv()) {
            log_warning("iidxio receiving failed");
            break;
        }

        // Keys/buttons

        uint16_t keys = iidx_io_ep2_get_keys();
        uint8_t panel = iidx_io_ep2_get_panel();
        uint8_t system = iidx_io_ep2_get_sys();

        _handle_buttons_14keys(keys, state);
        _handle_buttons_panel(panel, state);
        _handle_buttons_system(system, state);

        // Turntable

        uint8_t turntable[TURNTABLE_NUM];

        for (uint8_t i = 0; i < TURNTABLE_NUM; i++) {
            turntable[i] = iidx_io_ep2_get_turntable(i);
        }
        
        _handle_turntable(&config, turntable, state);

        // Pad update

        for (uint8_t i = 0; i < JOYSTICKS_NUM; i++) {
            vigem_target_x360_update(client, pad[i], state[i]);
        }

        // -----------------------------------------------------------------------------
        
        // Light related outputs

        if (config.cab_light.enable_keylight) {
            iidx_io_ep1_set_deck_lights(keys);
            iidx_io_ep1_set_panel_lights(panel);
        }

        if (config.cab_light.light_mode != LIGHT_SEQ_MODE_OFF) {
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

        if (config.cab_light.text_16seg[0] != '\0') {
            char buffer_16seg[9];

            vigem_iidxio_cab_16seg_sequencer_update(buffer_16seg);
            iidx_io_ep3_write_16seg(buffer_16seg);
        }

        if (!iidx_io_ep1_send()) {
            log_warning("iidxio sending failed");
            break;
        }
        
        if (_check_key(system, IIDX_IO_SYS_TEST) &&
                _check_key(system, IIDX_IO_SYS_SERVICE)) {
            log_info("Test + service pressed, exiting...");
            loop = false;
        }

        // Avoid CPU banging
        Sleep(1);
    }

    for (uint8_t i = 0; i < JOYSTICKS_NUM; i++) {
        vigem_target_remove(client, pad[i]);
        vigem_target_free(pad[i]);
    }

    vigem_free(client);

    Sleep(1000);

    _all_lights_off();

    iidx_io_fini();

    return 0;
}
