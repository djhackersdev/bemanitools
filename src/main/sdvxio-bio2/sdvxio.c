#define LOG_MODULE "sdvxio-bio2"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "api/core/config.h"
#include "api/core/log.h"

#include "iface-core/config.h"
#include "iface-core/log.h"

#include "sdk/module/core/log.h"
#include "sdk/module/io/sdvx.h"

#include "aciodrv/device.h"
#include "bio2drv/bi2a-sdvx.h"
#include "bio2drv/config.h"
#include "bio2drv/detect.h"

#include "sdk/module/core/config.h"
#include "sdk/module/configure.h"
#include "sdk/module/core/log.h"
#include "sdk/module/io/sdvx.h"

static bio2drv_config_t _sdvxio_bio2_config;

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

bool bt_io_sdvx_init()
{
    const char *selected_port = _sdvxio_bio2_config.port;

    if (_sdvxio_bio2_config.autodetect) {
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

    // BIO2's cannot share a bus with anything else, so use device directly
    bio2_device_ctx = aciodrv_device_open_path(selected_port, _sdvxio_bio2_config.baud);

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

void bt_io_sdvx_fini(void)
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

/* Blue generator lights are gpio, while the red and green are pwm on the KFCA.
   Tested this on a gen 1 sdvx pcb, and indeed the blue lights are only on/off,
   while the red & green can fade. */
void bt_io_sdvx_gpio_lights_set(uint32_t gpio_lights)
{
    pout_staging.gpio[0] = assign_light(gpio_lights, BT_IO_SDVX_OUT_GPIO_START);
    pout_staging.gpio[1] = assign_light(gpio_lights, BT_IO_SDVX_OUT_GPIO_A);
    pout_staging.gpio[2] = assign_light(gpio_lights, BT_IO_SDVX_OUT_GPIO_B);
    pout_staging.gpio[3] = assign_light(gpio_lights, BT_IO_SDVX_OUT_GPIO_C);
    pout_staging.gpio[4] = assign_light(gpio_lights, BT_IO_SDVX_OUT_GPIO_D);
    pout_staging.gpio[5] = assign_light(gpio_lights, BT_IO_SDVX_OUT_GPIO_FX_L);
    pout_staging.gpio[6] = assign_light(gpio_lights, BT_IO_SDVX_OUT_GPIO_FX_R);
    pout_staging.generator[2] =
        assign_light(gpio_lights, BT_IO_SDVX_OUT_GPIO_GENERATOR_B);
}

void bt_io_sdvx_pwm_light_set(uint8_t light_no, uint8_t intensity)
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
            case 18:
                pout_staging.generator[0] = intensity;
                break;
            case 19:
                pout_staging.generator[1] = intensity;
                break;
            default:
                break;
        }
    }
}

bool bt_io_sdvx_output_write(void)
{
    memcpy(&pout_ready, &pout_staging, sizeof(struct bi2a_sdvx_state_out));
    pout_ready.wingUpper[0] = wing_staging[0] / 2 + wing_staging[3] / 2;
    pout_ready.wingUpper[1] = wing_staging[1] / 2 + wing_staging[4] / 2;
    pout_ready.wingUpper[2] = wing_staging[2] / 2 + wing_staging[5] / 2;

    pout_ready.wingLower[0] = wing_staging[6] / 2 + wing_staging[9] / 2;
    pout_ready.wingLower[1] = wing_staging[7] / 2 + wing_staging[10] / 2;
    pout_ready.wingLower[2] = wing_staging[8] / 2 + wing_staging[11] / 2;

    pout_ready.gpio2[0] = 127;
    pout_ready.gpio2[1] = 127;
    pout_ready.gpio2[2] = 127;

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

bool bt_io_sdvx_input_read(void)
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
        shift_pin(pin.analogs[0].a_coin, BT_IO_SDVX_IN_GPIO_SYS_COIN);
    sdvx_io_gpio_sys |=
        shift_pin(pin.analogs[0].a_test, BT_IO_SDVX_IN_GPIO_SYS_TEST);
    sdvx_io_gpio_sys |=
        shift_pin(pin.analogs[0].a_service, BT_IO_SDVX_IN_GPIO_SYS_SERVICE);

    sdvx_io_gpio[0] = 0;
    sdvx_io_gpio[1] = 0;

    sdvx_io_gpio[0] |=
        shift_pin(pin.buttons_1.b_start, BT_IO_SDVX_IN_GPIO_0_START);
    sdvx_io_gpio[0] |=
        shift_pin(pin.buttons_1.b_headphone, BT_IO_SDVX_IN_GPIO_0_HEADPHONE);
    sdvx_io_gpio[0] |= shift_pin(pin.buttons_1.b_a, BT_IO_SDVX_IN_GPIO_0_A);
    sdvx_io_gpio[0] |= shift_pin(pin.buttons_1.b_b, BT_IO_SDVX_IN_GPIO_0_B);
    sdvx_io_gpio[0] |= shift_pin(pin.buttons_1.b_c, BT_IO_SDVX_IN_GPIO_0_C);
    sdvx_io_gpio[1] |= shift_pin(pin.buttons_1.b_d, BT_IO_SDVX_IN_GPIO_1_D);
    sdvx_io_gpio[1] |=
        shift_pin(pin.buttons_1.b_fxl, BT_IO_SDVX_IN_GPIO_1_FX_L);
    sdvx_io_gpio[1] |=
        shift_pin(pin.buttons_2.b_fxr, BT_IO_SDVX_IN_GPIO_1_FX_R);

    return true;
}

uint8_t bt_io_sdvx_input_gpio_sys_get(void)
{
    return sdvx_io_gpio_sys;
}

uint16_t bt_io_sdvx_input_gpio_get(uint8_t gpio_bank)
{
    if (gpio_bank > 1) {
        return 0;
    }

    return sdvx_io_gpio[gpio_bank];
}

uint16_t bt_io_sdvx_spinner_pos_get(uint8_t spinner_no)
{
    if (spinner_no >= 2) {
        return 0;
    }
    return sdvx_io_analog[spinner_no];
}

bool bt_io_sdvx_amp_volume_set(
    uint8_t primary, uint8_t headphone, uint8_t subwoofer)
{
    if (!running) {
        return false;
    }

    // yes, the BIO2 doesn't allow control of the amps individually
    // so let's just set it so that people's ear's don't blow out
    if (!bio2drv_bi2a_sdvx_amp(
            bio2_device_ctx, bio2_node_id, 0, 0, primary, primary)) {
        return false;
    }

    return true;
}

void bt_module_core_config_api_set(const bt_core_config_api_t *api)
{
    bt_core_config_api_set(api);
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

bool bt_module_configure_do(const bt_core_config_t *config)
{
    bio2drv_config_bio2_get(config, &_sdvxio_bio2_config);

    return true;
}

void bt_module_io_sdvx_api_get(bt_io_sdvx_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_sdvx_init;
    api->v1.fini = bt_io_sdvx_fini;
    api->v1.gpio_lights_set = bt_io_sdvx_gpio_lights_set;
    api->v1.pwm_light_set = bt_io_sdvx_pwm_light_set;
    api->v1.output_write = bt_io_sdvx_output_write;
    api->v1.input_read = bt_io_sdvx_input_read;
    api->v1.input_gpio_sys_get = bt_io_sdvx_input_gpio_sys_get;
    api->v1.input_gpio_get = bt_io_sdvx_input_gpio_get;
    api->v1.spinner_pos_get = bt_io_sdvx_spinner_pos_get;
    api->v1.amp_volume_set = bt_io_sdvx_amp_volume_set;
}