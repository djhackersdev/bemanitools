#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "bemanitools/glue.h"
#include "bemanitools/sdvxio.h"

#include "cconfig/cconfig-main.h"

#include "aciodrv/device.h"
#include "bio2drv/bi2a-sdvx.h"
#include "bio2drv/config-bio2.h"
#include "bio2drv/detect.h"

#define LOG_MODULE "sdvxio-bio2"

#define log_misc(...) sdvx_io_log_misc(LOG_MODULE, __VA_ARGS__)
#define log_info(...) sdvx_io_log_info(LOG_MODULE, __VA_ARGS__)
#define log_warning(...) sdvx_io_log_warning(LOG_MODULE, __VA_ARGS__)
#define log_fatal(...) sdvx_io_log_fatal(LOG_MODULE, __VA_ARGS__)

static log_formatter_t sdvx_io_log_misc;
static log_formatter_t sdvx_io_log_info;
static log_formatter_t sdvx_io_log_warning;
static log_formatter_t sdvx_io_log_fatal;

static uint16_t sdvx_io_gpio[2];
static uint8_t sdvx_io_gpio_sys;
static uint16_t sdvx_io_analog[2];

static char autodetect_buffer[512];

static atomic_bool running;
static atomic_bool processing_io;
static int16_t bio2_node_id;

uint8_t wing_staging[12];
struct bi2a_sdvx_state_out pout_staging;
struct bi2a_sdvx_state_out pout_ready;

static struct aciodrv_device_ctx *bio2_device_ctx;

void sdvx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    sdvx_io_log_misc = misc;
    sdvx_io_log_info = info;
    sdvx_io_log_warning = warning;
    sdvx_io_log_fatal = fatal;

    bio2drv_set_loggers(misc, info, warning, fatal);
}

bool sdvx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{

    struct cconfig *config;
    struct bio2drv_config_bio2 config_bio2;

    config = cconfig_init();

    bio2drv_config_bio2_init(config);

    if (!cconfig_main_config_init(
            config,
            "--bio2-config",
            "sdvxio-bio2.conf",
            "--help",
            "-h",
            "sdvxio-bio2",
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    bio2drv_config_bio2_get(&config_bio2, config);

    cconfig_finit(config);

    const char *selected_port = config_bio2.port;

    if (config_bio2.autodetect) {
        log_info("Attempting autodetect");

        if (bio2drv_detect(
                DETECT_DEVICEID,
                0,
                autodetect_buffer,
                sizeof(autodetect_buffer))) {
            selected_port = autodetect_buffer;
        } else {
            log_info("Autodetect failed, falling back to using specified port");
        }
    }

    bio2_device_ctx = aciodrv_device_open(selected_port, config_bio2.baud);

    if (bio2_device_ctx == NULL) {
        log_info("Opening BIO2 device on [%s] failed", selected_port);
        return 0;
    }

    log_info("Opening BIO2 device on [%s] successful", selected_port);

    uint8_t node_count = aciodrv_device_get_node_count(bio2_device_ctx);
    log_info("Enumerated %d nodes", node_count);

    bio2_node_id = -1;

    for (uint8_t i = 0; i < node_count; i++) {
        char product[4];
        aciodrv_device_get_node_product_ident(bio2_device_ctx, i, product);
        log_info(
            "> %d: %c%c%c%c\n",
            i,
            product[0],
            product[1],
            product[2],
            product[3]);

        if (!memcmp(product, "BI2A", 4)) {
            if (bio2_node_id != -1) {
                log_warning("Multiple BI2A found! Using highest node id.");
            }
            bio2_node_id = i;
        }
    }

    if (bio2_node_id != -1) {
        log_warning("Using BI2A on node: %d", bio2_node_id);

        if (!bio2drv_bi2a_sdvx_init(bio2_device_ctx, bio2_node_id)) {
            log_warning("Unable to start BI2A on node: %d", bio2_node_id);
            return false;
        }

        running = true;
        log_warning("sdvxio-bio2 now running");
    } else {
        log_warning("No KFCA device found");
    }

    return running;
}

void sdvx_io_fini(void)
{
    running = false;
    while (processing_io) {
        // avoid banging
        Sleep(1);
    }
}

static uint8_t assign_light(uint32_t gpio_lights, uint32_t shift)
{
    uint32_t shifted = (1 << shift);

    if ((gpio_lights & shifted) == shifted) {
        return 255;
    }

    return 0;
}

void sdvx_io_set_gpio_lights(uint32_t gpio_lights)
{
    pout_staging.gpio[0] = assign_light(gpio_lights, SDVX_IO_OUT_GPIO_START);
    pout_staging.gpio[1] = assign_light(gpio_lights, SDVX_IO_OUT_GPIO_A);
    pout_staging.gpio[2] = assign_light(gpio_lights, SDVX_IO_OUT_GPIO_B);
    pout_staging.gpio[3] = assign_light(gpio_lights, SDVX_IO_OUT_GPIO_C);
    pout_staging.gpio[4] = assign_light(gpio_lights, SDVX_IO_OUT_GPIO_D);
    pout_staging.gpio[5] = assign_light(gpio_lights, SDVX_IO_OUT_GPIO_FX_L);
    pout_staging.gpio[6] = assign_light(gpio_lights, SDVX_IO_OUT_GPIO_FX_R);
}

void sdvx_io_set_pwm_light(uint8_t light_no, uint8_t intensity)
{
    if (light_no < 12) {
        wing_staging[light_no] = intensity;
    } else {
        switch (light_no) {
            case 12:
                pout_staging.woof_r = intensity;
                break;
            case 13:
                pout_staging.woof_g = intensity;
                break;
            case 14:
                pout_staging.woof_b = intensity;
                break;
            case 15:
                pout_staging.controller[0] = intensity;
                break;
            case 16:
                pout_staging.controller[1] = intensity;
                break;
            case 17:
                pout_staging.controller[2] = intensity;
                break;
            default:
                break;
        }
    }
}

bool sdvx_io_write_output(void)
{
    memcpy(&pout_ready, &pout_staging, sizeof(struct bi2a_sdvx_state_out));
    pout_ready.wingUpper[0] = wing_staging[0] / 2 + wing_staging[3] / 2;
    pout_ready.wingUpper[1] = wing_staging[1] / 2 + wing_staging[4] / 2;
    pout_ready.wingUpper[2] = wing_staging[2] / 2 + wing_staging[5] / 2;

    pout_ready.wingLower[0] = wing_staging[6] / 2 + wing_staging[9] / 2;
    pout_ready.wingLower[1] = wing_staging[7] / 2 + wing_staging[10] / 2;
    pout_ready.wingLower[2] = wing_staging[8] / 2 + wing_staging[11] / 2;

    return true;
}

static uint8_t shift_pin(uint16_t value, uint8_t pin)
{
    if (value) {
        return (1 << pin);
    }

    return 0;
}

bool _bio2_sdvx_io_poll(
    const struct bi2a_sdvx_state_out *pout, struct bi2a_sdvx_state_in *pin)
{
    if (!running) {
        return false;
    }

    processing_io = true;

    if (!bio2drv_bi2a_sdvx_poll(bio2_device_ctx, bio2_node_id, pout, pin)) {
        processing_io = false;
        return false;
    }

    processing_io = false;
    return true;
}

bool sdvx_io_read_input(void)
{
    struct bi2a_sdvx_state_in pin;

    if (!_bio2_sdvx_io_poll(&pout_ready, &pin)) {
        return false;
    }

    pin.raw[0] = ac_io_u16(pin.raw[0]);
    pin.raw[1] = ac_io_u16(pin.raw[1]);

    sdvx_io_analog[0] = pin.analogs[0].a_val;
    sdvx_io_analog[1] = pin.analogs[1].a_val;

    sdvx_io_gpio_sys = 0;
    sdvx_io_gpio_sys |=
        shift_pin(pin.analogs[0].a_coin, SDVX_IO_IN_GPIO_SYS_COIN);
    sdvx_io_gpio_sys |=
        shift_pin(pin.analogs[0].a_test, SDVX_IO_IN_GPIO_SYS_TEST);
    sdvx_io_gpio_sys |=
        shift_pin(pin.analogs[0].a_service, SDVX_IO_IN_GPIO_SYS_SERVICE);

    sdvx_io_gpio[0] = 0;
    sdvx_io_gpio[1] = 0;

    sdvx_io_gpio[0] |=
        shift_pin(pin.buttons_1.b_start, SDVX_IO_IN_GPIO_0_START);
    sdvx_io_gpio[0] |=
        shift_pin(pin.buttons_1.b_headphone, SDVX_IO_IN_GPIO_0_HEADPHONE);
    sdvx_io_gpio[0] |= shift_pin(pin.buttons_1.b_a, SDVX_IO_IN_GPIO_0_A);
    sdvx_io_gpio[0] |= shift_pin(pin.buttons_1.b_b, SDVX_IO_IN_GPIO_0_B);
    sdvx_io_gpio[0] |= shift_pin(pin.buttons_1.b_c, SDVX_IO_IN_GPIO_0_C);
    sdvx_io_gpio[1] |= shift_pin(pin.buttons_1.b_d, SDVX_IO_IN_GPIO_1_D);
    sdvx_io_gpio[1] |= shift_pin(pin.buttons_1.b_fxl, SDVX_IO_IN_GPIO_1_FX_L);
    sdvx_io_gpio[1] |= shift_pin(pin.buttons_2.b_fxr, SDVX_IO_IN_GPIO_1_FX_R);

    return true;
}

uint8_t sdvx_io_get_input_gpio_sys(void)
{
    return sdvx_io_gpio_sys;
}

uint16_t sdvx_io_get_input_gpio(uint8_t gpio_bank)
{
    if (gpio_bank > 1) {
        return 0;
    }

    return sdvx_io_gpio[gpio_bank];
}

uint16_t sdvx_io_get_spinner_pos(uint8_t spinner_no)
{
    if (spinner_no >= 2) {
        return 0;
    }
    return sdvx_io_analog[spinner_no];
}

bool sdvx_io_set_amp_volume(
    uint8_t primary, uint8_t headphone, uint8_t subwoofer)
{
    if (!running) {
        return false;
    }

    // yes, the BIO2 doesn't allow control of the amps individually
    // so let's just set it so that people's ear's don't blow out
    if (!bio2drv_bi2a_sdvx_amp(bio2_device_ctx, bio2_node_id, 0, 0, primary, primary)) {
        return false;
    }

    return true;
}
