#include <windows.h>

#define LOG_MODULE "iidxio-lightning"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "bemanitools/glue.h"
#include "bemanitools/sdvxio.h"

#include "util/log.h"

#include "load_aio.h"

static atomic_bool running;
static atomic_bool processing_io;

static struct AIO_IOB2_BI2X_TBS__DEVSTATUS pin_cur;

static struct bi2x_ctx *bi2x_ctx;


static bool _bio2_iidx_io_poll(struct AIO_IOB2_BI2X_TBS__DEVSTATUS *pin)
{
    if (!running) {
        return false;
    }

    processing_io = true;

    poll_bi2x(bi2x_ctx, pin);

    processing_io = false;
    return true;
}

void sdvx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    log_to_external(misc, info, warning, fatal);
}

bool sdvx_io_init(
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

void sdvx_io_fini(void)
{
    running = false;

    while (processing_io) {
        // avoid banging
        Sleep(1);
    }

    close_bi2x(bi2x_ctx);

    bi2x_ctx = NULL;
}

void sdvx_io_set_gpio_lights(uint32_t gpio_lights)
{
    // wow ignore
    return;
}

void sdvx_io_set_pwm_light(uint8_t light_no, uint8_t intensity)
{
    // wow ignore
    return;
}

bool sdvx_io_write_output(void)
{
    return true;
}

bool sdvx_io_read_input(void)
{
    if (!_bio2_iidx_io_poll(&pin_cur)) {
        return false;
    }

    return true;
}

uint16_t sdvx_io_get_spinner_pos(uint8_t spinner_no)
{
    // spinner_no
    int32_t raw_value = 0;
    if (spinner_no == 0) {
        raw_value = pin_cur.Input.StickX;
    }
    if (spinner_no == 1) {
        raw_value = pin_cur.Input.StickY;
    }
    int32_t value = raw_value - 32767;

    float converted = value / 32767.0f;
    
    converted *= 2.0;

    if (converted > 1.0f) {
        converted = 1.0f;
    } else if (converted < -1.0f) {
        converted = -1.0f;
    }

    int32_t final = - 32767 * converted;

    return (uint16_t)final/64;
}

static uint8_t shift_pin(uint16_t value, uint8_t pin)
{
    if (value) {
        return (1 << pin);
    }

    return 0;
}

uint8_t sdvx_io_get_input_gpio_sys(void)
{
    return 0;
}

uint16_t sdvx_io_get_input_gpio(uint8_t gpio_bank)
{
    if (gpio_bank > 2) {
        return 0;
    }

    uint16_t data = 0;

    if (gpio_bank == 0) {
        data |= shift_pin(pin_cur.Input.bStickBtn, SDVX_IO_IN_GPIO_0_START);
        data |= shift_pin(pin_cur.Input.bButton0, SDVX_IO_IN_GPIO_0_A);
        data |= shift_pin(pin_cur.Input.bButton1, SDVX_IO_IN_GPIO_0_B);
        data |= shift_pin(pin_cur.Input.bButton2, SDVX_IO_IN_GPIO_0_C);
    } else if (gpio_bank == 1) {
        data |= shift_pin(pin_cur.Input.bButton3, SDVX_IO_IN_GPIO_1_D);
        data |= shift_pin(pin_cur.Input.bTrigger1, SDVX_IO_IN_GPIO_1_FX_L);
        data |= shift_pin(pin_cur.Input.bTrigger2, SDVX_IO_IN_GPIO_1_FX_R);
    }

    return data;
}

bool sdvx_io_set_amp_volume(
    uint8_t primary, uint8_t headphone, uint8_t subwoofer)
{
    return true;
}
