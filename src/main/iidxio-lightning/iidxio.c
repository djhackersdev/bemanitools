#include <windows.h>

#define LOG_MODULE "iidxio-lightning"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "bemanitools/glue.h"
#include "bemanitools/iidxio.h"

#include "util/log.h"

#include "load_aio.h"

static atomic_bool running;
static atomic_bool processing_io;

static struct AIO_IOB2_BI2X_TDJ__DEVSTATUS pin_cur;

static struct bi2x_ctx *bi2x_ctx;


static bool _bio2_iidx_io_poll(struct AIO_IOB2_BI2X_TDJ__DEVSTATUS *pin)
{
    if (!running) {
        return false;
    }

    processing_io = true;

    poll_bi2x(bi2x_ctx, pin);

    processing_io = false;
    return true;
}

void iidx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    log_to_external(misc, info, warning, fatal);
}

bool iidx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    init_aio_handles();

    bi2x_ctx = setup_bi2x();
    if (!bi2x_ctx) {
        log_warning("Unable to start BI2X?");
        return false;
    }
    running = true;

    return running;
}

void iidx_io_fini(void)
{
    running = false;

    while (processing_io) {
        // avoid banging
        Sleep(1);
    }

    close_bi2x(bi2x_ctx);

    bi2x_ctx = NULL;
}

void iidx_io_ep1_set_deck_lights(uint16_t deck_lights)
{
    return;
}

void iidx_io_ep1_set_panel_lights(uint8_t panel_lights)
{
    return;
}

void iidx_io_ep1_set_top_lamps(uint8_t top_lamps)
{
    return;
}

void iidx_io_ep1_set_top_neons(bool top_neons)
{
    return;
}

bool iidx_io_ep1_send(void)
{
    return true;
}

bool iidx_io_ep2_recv(void)
{
    if (!_bio2_iidx_io_poll(&pin_cur)) {
        return false;
    }

    return true;
}

uint8_t iidx_io_ep2_get_turntable(uint8_t player_no)
{
    switch (player_no) {
        case 0:
            return pin_cur.a_turntable[0];
        case 1:
            return pin_cur.a_turntable[1];
        default:
            return 0;
    }
}

uint8_t iidx_io_ep2_get_slider(uint8_t slider_no)
{
    return 0;
}

uint8_t iidx_io_ep2_get_sys(void)
{
    uint8_t state;

    state = 0;

    if (pin_cur.b_test) {
        state |= (1 << IIDX_IO_SYS_TEST);
    }

    if (pin_cur.b_service) {
        state |= (1 << IIDX_IO_SYS_SERVICE);
    }

    if (pin_cur.b_coinmech) {
        state |= (1 << IIDX_IO_SYS_COIN);
    }

    return state;
}

uint8_t iidx_io_ep2_get_panel(void)
{
    uint8_t state;

    state = 0;

    if (pin_cur.b_start[0]) {
        state |= (1 << IIDX_IO_PANEL_P1_START);
    }

    if (pin_cur.b_start[1]) {
        state |= (1 << IIDX_IO_PANEL_P2_START);
    }

    if (pin_cur.b_vefx) {
        state |= (1 << IIDX_IO_PANEL_VEFX);
    }

    if (pin_cur.b_effect) {
        state |= (1 << IIDX_IO_PANEL_EFFECT);
    }

    return state;
}

uint16_t iidx_io_ep2_get_keys(void)
{
    uint16_t state;

    state = 0;

    state |= ((pin_cur.b_p1[0] > 0) << IIDX_IO_KEY_P1_1);
    state |= ((pin_cur.b_p1[1] > 0) << IIDX_IO_KEY_P1_2);
    state |= ((pin_cur.b_p1[2] > 0) << IIDX_IO_KEY_P1_3);
    state |= ((pin_cur.b_p1[3] > 0) << IIDX_IO_KEY_P1_4);
    state |= ((pin_cur.b_p1[4] > 0) << IIDX_IO_KEY_P1_5);
    state |= ((pin_cur.b_p1[5] > 0) << IIDX_IO_KEY_P1_6);
    state |= ((pin_cur.b_p1[6] > 0) << IIDX_IO_KEY_P1_7);

    state |= ((pin_cur.b_p2[0] > 0) << IIDX_IO_KEY_P2_1);
    state |= ((pin_cur.b_p2[1] > 0) << IIDX_IO_KEY_P2_2);
    state |= ((pin_cur.b_p2[2] > 0) << IIDX_IO_KEY_P2_3);
    state |= ((pin_cur.b_p2[3] > 0) << IIDX_IO_KEY_P2_4);
    state |= ((pin_cur.b_p2[4] > 0) << IIDX_IO_KEY_P2_5);
    state |= ((pin_cur.b_p2[5] > 0) << IIDX_IO_KEY_P2_6);
    state |= ((pin_cur.b_p2[6] > 0) << IIDX_IO_KEY_P2_7);

    return state;
}

bool iidx_io_ep3_write_16seg(const char *text)
{
    return true;
}
