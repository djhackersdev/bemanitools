#define LOG_MODULE "mode-test"

#include <stdio.h>
#include <stdlib.h>

#include "extiodrv/extio.h"

#include "iface-core/log.h"

#include "p3io/ddr.h"
#include "p3iodrv/ddr.h"
#include "p3iodrv/device.h"

#include "mode-test.h"

struct mode_test_output_state {
    struct p3io_ddr_output output;
    struct extiodrv_extio_pad_lights pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS];
    bool neons;
};

struct mode_test_input_state {
    struct p3io_ddr_jamma jamma[4];
};

static bool _check_input_state_change(
    const struct mode_test_input_state *cur_state,
    const struct mode_test_input_state *prev_state)
{
    return !memcmp(cur_state, prev_state, sizeof(struct mode_test_input_state));
}

static void _input_state_swap(
    struct mode_test_input_state **cur_state,
    struct mode_test_input_state **prev_state)
{
    struct mode_test_input_state *tmp;

    tmp = *cur_state;
    *cur_state = *prev_state;
    *prev_state = tmp;
}

static bool _update_io_frame(
    HANDLE handle_p3io,
    HANDLE handle_extio,
    bool polling_mode,
    struct mode_test_input_state *input,
    const struct mode_test_output_state *output)
{
    HRESULT hr;

    hr = p3iodrv_ddr_set_outputs(handle_p3io, &output->output);

    if (FAILED(hr)) {
        log_warning("Setting outputs failed: %lX", hr);
        return false;
    }

    if (polling_mode) {
        hr = extiodrv_extio_transfer(
            handle_extio,
            EXTIODRV_EXTIO_SENSOR_READ_MODE_ALL,
            output->pad_lights,
            output->neons);

        if (FAILED(hr)) {
            log_warning("extio transfer failed: %lX", hr);
            return false;
        }

        hr = p3iodrv_ddr_get_jamma(handle_p3io, &input->jamma[0]);

        if (FAILED(hr)) {
            log_warning("Reading jamma failed: %lX", hr);
            return false;
        }
    } else {
        for (uint8_t i = 0; i < 4; i++) {
            hr = extiodrv_extio_transfer(
                handle_extio, i + 1, output->pad_lights, output->neons);

            if (FAILED(hr)) {
                log_warning("extio transfer failed: %lX", hr);
                return false;
            }

            hr = p3iodrv_ddr_get_jamma(handle_p3io, &input->jamma[i]);

            if (FAILED(hr)) {
                log_warning("Reading jamma failed: %lX", hr);
                return false;
            }
        }
    }

    return true;
}

static void _set_outputs_on_inputs(
    const struct mode_test_input_state *input,
    struct mode_test_output_state *output)
{
    memset(&output->pad_lights, 0, sizeof(output->pad_lights));

    output->output.cabinet.p1_menu_buttons = 0;
    output->output.cabinet.p2_menu_buttons = 0;

    for (uint8_t i = 0; i < 4; i++) {
        output->pad_lights[0].up |= input->jamma[i].p1.pad_up ? 1 : 0;
        output->pad_lights[0].down |= input->jamma[i].p1.pad_down ? 1 : 0;
        output->pad_lights[0].left |= input->jamma[i].p1.pad_left ? 1 : 0;
        output->pad_lights[0].right |= input->jamma[i].p1.pad_right ? 1 : 0;

        output->pad_lights[1].up |= input->jamma[i].p2.pad_up ? 1 : 0;
        output->pad_lights[1].down |= input->jamma[i].p2.pad_down ? 1 : 0;
        output->pad_lights[1].left |= input->jamma[i].p2.pad_left ? 1 : 0;
        output->pad_lights[1].right |= input->jamma[i].p2.pad_right ? 1 : 0;

        output->output.cabinet.p1_menu_buttons |=
            (input->jamma[i].p1.menu_left || input->jamma[i].p1.menu_right ||
             input->jamma[i].p1.menu_start) ?
            1 :
            0;
        output->output.cabinet.p2_menu_buttons |=
            (input->jamma[i].p2.menu_left || input->jamma[i].p2.menu_right ||
             input->jamma[i].p2.menu_start) ?
            1 :
            0;
    }
}

static void _render_main(
    uint8_t cnt, bool polling_mode, const struct mode_test_input_state *state)
{
    printf("Counter: %d\n", cnt);
    printf("Polling mode: %d\n", polling_mode);
    printf("Player 1       Player 2\n");
    printf("\n");
    printf("Pad\n");

    if (polling_mode) {
        printf(
            "Up:    %d       Up:    %d\n",
            state->jamma[0].p1.pad_up,
            state->jamma[0].p2.pad_up);
        printf(
            "Down:  %d       Down:  %d\n",
            state->jamma[0].p1.pad_down,
            state->jamma[0].p2.pad_down);
        printf(
            "Left:  %d       Left:  %d\n",
            state->jamma[0].p1.pad_left,
            state->jamma[0].p2.pad_left);
        printf(
            "Right: %d       Right: %d\n",
            state->jamma[0].p1.pad_right,
            state->jamma[0].p2.pad_right);
    } else {
        printf(
            "Up:    %c%c%c%c   Up:    %c%c%c%c\n",
            state->jamma[0].p1.pad_up ? 'U' : '_',
            state->jamma[1].p1.pad_up ? 'D' : '_',
            state->jamma[2].p1.pad_up ? 'L' : '_',
            state->jamma[3].p1.pad_up ? 'R' : '_',
            state->jamma[0].p2.pad_up ? 'U' : '_',
            state->jamma[1].p2.pad_up ? 'D' : '_',
            state->jamma[2].p2.pad_up ? 'L' : '_',
            state->jamma[3].p2.pad_up) ?
            'R' :
            '_';
        printf(
            "Down:  %c%c%c%c   Down:  %c%c%c%c\n",
            state->jamma[0].p1.pad_down ? 'U' : '_',
            state->jamma[1].p1.pad_down ? 'D' : '_',
            state->jamma[2].p1.pad_down ? 'L' : '_',
            state->jamma[3].p1.pad_down ? 'R' : '_',
            state->jamma[0].p2.pad_down ? 'U' : '_',
            state->jamma[1].p2.pad_down ? 'D' : '_',
            state->jamma[2].p2.pad_down ? 'L' : '_',
            state->jamma[3].p2.pad_down ? 'R' : '_');
        printf(
            "Left:  %c%c%c%c   Left:  %c%c%c%c\n",
            state->jamma[0].p1.pad_left ? 'U' : '_',
            state->jamma[1].p1.pad_left ? 'D' : '_',
            state->jamma[2].p1.pad_left ? 'L' : '_',
            state->jamma[3].p1.pad_left ? 'R' : '_',
            state->jamma[0].p2.pad_left ? 'U' : '_',
            state->jamma[1].p2.pad_left ? 'D' : '_',
            state->jamma[2].p2.pad_left ? 'L' : '_',
            state->jamma[3].p2.pad_left ? 'R' : '_');
        printf(
            "Right: %c%c%c%c   Right: %c%c%c%c\n",
            state->jamma[0].p1.pad_right ? 'U' : '_',
            state->jamma[1].p1.pad_right ? 'D' : '_',
            state->jamma[2].p1.pad_right ? 'L' : '_',
            state->jamma[3].p1.pad_right ? 'R' : '_',
            state->jamma[0].p2.pad_right ? 'U' : '_',
            state->jamma[1].p2.pad_right ? 'D' : '_',
            state->jamma[2].p2.pad_right ? 'L' : '_',
            state->jamma[3].p2.pad_right ? 'R' : '_');
    }

    printf("\n");
    printf("Menu\n");
    printf(
        "Start: %d       Start: %d\n",
        state->jamma[0].p1.menu_start,
        state->jamma[0].p2.menu_start);
    printf(
        "Up:    %d       Up:    %d\n",
        state->jamma[0].operator.p1_menu_up,
        state->jamma[0].operator.p2_menu_up);
    printf(
        "Down:  %d       Down:  %d\n",
        state->jamma[0].operator.p1_menu_down,
        state->jamma[0].operator.p2_menu_down);
    printf(
        "Left:  %d       Left:  %d\n",
        state->jamma[0].p1.menu_left,
        state->jamma[0].p2.menu_left);
    printf(
        "Right: %d       Right: %d\n",
        state->jamma[0].p1.menu_right,
        state->jamma[0].p2.menu_right);
    printf("\n");
    printf("Operator\n");
    printf(
        "Test: %d   Service: %d   Coin: %d\n",
        state->jamma[0].operator.test,
        state->jamma[0].operator.service,
        state->jamma[0].operator.coin);
}

static void
_render_menu(bool *continue_loop, struct mode_test_output_state *output)
{
    printf(
        "Menu options:\n"
        "  0: Exit menu and continue loop\n"
        "  1: Exit\n"
        "  2: Set neon state\n"
        "  3: Top lamp state\n"
        "  4: Set all lights on\n"
        "  5: Set all lights off\n"
        "Waiting for input: ");

    char c = getchar();

    // Keyboard input debounce
    Sleep(10);

    switch (c) {
        case '1': {
            *continue_loop = false;
            break;
        }

        case '2': {
            int state;
            int n;

            printf("Enter neon state (0/1): ");

            n = scanf("%d", &state);

            if (n > 0) {
                output->neons = state > 0;
            }

            break;
        }

        case '3': {
            char buf[4];
            int n;

            printf("Enter top lamp state, chain of 0/1s: ");

            n = scanf("%8s", buf);

            if (n > 0) {
                output->output.cabinet.top_p1_upper = buf[0] == '1';
                output->output.cabinet.top_p1_lower = buf[1] == '1';
                output->output.cabinet.top_p2_upper = buf[2] == '1';
                output->output.cabinet.top_p2_lower = buf[3] == '1';
            }

            break;
        }

        case '4': {
            output->output.cabinet.top_p1_upper = 1;
            output->output.cabinet.top_p1_lower = 1;
            output->output.cabinet.top_p2_upper = 1;
            output->output.cabinet.top_p2_lower = 1;

            output->output.cabinet.p1_menu_buttons = 1;
            output->output.cabinet.p2_menu_buttons = 1;

            output->neons = true;

            output->pad_lights[0].up = true;
            output->pad_lights[0].down = true;
            output->pad_lights[0].left = true;
            output->pad_lights[0].right = true;

            output->pad_lights[1].up = true;
            output->pad_lights[1].down = true;
            output->pad_lights[1].left = true;
            output->pad_lights[1].right = true;

            break;
        }

        case '5': {
            output->output.cabinet.top_p1_upper = 0;
            output->output.cabinet.top_p1_lower = 0;
            output->output.cabinet.top_p2_upper = 0;
            output->output.cabinet.top_p2_lower = 0;

            output->output.cabinet.p1_menu_buttons = 0;
            output->output.cabinet.p2_menu_buttons = 0;

            output->neons = false;

            output->pad_lights[0].up = false;
            output->pad_lights[0].down = false;
            output->pad_lights[0].left = false;
            output->pad_lights[0].right = false;

            output->pad_lights[1].up = false;
            output->pad_lights[1].down = false;
            output->pad_lights[1].left = false;
            output->pad_lights[1].right = false;

            break;
        }

        case '0':
        default:
            break;
    }
}

bool mode_test_proc(HANDLE handle_p3io, HANDLE handle_extio)
{
    struct mode_test_input_state state[2];
    struct mode_test_input_state *cur_input_state;
    struct mode_test_input_state *prev_input_state;
    struct mode_test_output_state output_state;

    bool polling_mode;
    bool loop;
    uint8_t cnt;

    memset(&state, 0, sizeof(state));
    cur_input_state = &state[0];
    prev_input_state = &state[1];
    memset(&output_state, 0, sizeof(output_state));

    polling_mode = true;
    loop = true;
    cnt = 0;

    fprintf(stderr, ">>> Press enter to start running test mode <<<\n");

    if (getchar() != '\n') {
        return true;
    }

    while (loop) {
        if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
            system("cls");

            _render_menu(&loop, &output_state);
        } else {
            _input_state_swap(&cur_input_state, &prev_input_state);

            memset(cur_input_state, 0, sizeof(struct mode_test_input_state));

            if (!_update_io_frame(
                    handle_p3io,
                    handle_extio,
                    polling_mode,
                    cur_input_state,
                    &output_state)) {
                return false;
            }

            _set_outputs_on_inputs(cur_input_state, &output_state);

            if (_check_input_state_change(cur_input_state, prev_input_state)) {
                system("cls");

                _render_main(cnt, polling_mode, cur_input_state);
            }
        }

        /* avoid CPU banging */
        Sleep(1);
        ++cnt;
    }

    return true;
}