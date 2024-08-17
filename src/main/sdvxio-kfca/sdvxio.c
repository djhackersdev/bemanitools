#define LOG_MODULE "sdvxio-kfca"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "acio/acio.h"

#include "api/core/config.h"
#include "api/core/log.h"

#include "iface-core/config.h"
#include "iface-acio/mgr.h"
#include "iface-core/log.h"

#include "main/module/acio-mgr-ext.h"

#include "sdk/module/core/config.h"
#include "sdk/module/configure.h"
#include "sdk/module/core/log.h"
#include "sdk/module/io/sdvx.h"

#include "aciodrv/kfca.h"

#include "sdvxio-kfca/config-kfca.h"

static sdvxio_kfca_config_kfca_t _sdvxio_kfca_config;

static uint16_t sdvx_io_gpio[2];
static uint8_t sdvx_io_gpio_sys;
static uint16_t sdvx_io_analog[2];

static atomic_bool running;
static atomic_bool processing_io;
static int16_t kfca_node_id;

struct ac_io_kfca_poll_out pout_staging;
struct ac_io_kfca_poll_out pout_ready;

static bt_acio_mgr_port_dispatcher_t *acio_manager_ctx;

static module_acio_mgr_t *acio_mgr_module;

static void _bt_io_jb_module_acio_mgr_init(module_acio_mgr_t **module)
{
    bt_acio_mgr_api_t api;

    module_acio_mgr_ext_load_and_init("acio-mgr.dll", module);
    module_acio_mgr_api_get(*module, &api);
    bt_acio_mgr_api_set(&api);
}

bool bt_io_sdvx_init()
{
    _bt_io_jb_module_acio_mgr_init(&acio_mgr_module);

    acio_manager_ctx =
        bt_acio_mgr_port_init(_sdvxio_kfca_config.port, _sdvxio_kfca_config.baud);

    if (acio_manager_ctx == NULL) {
        log_info("Opening acio device on [%s] failed", _sdvxio_kfca_config.port);
        return 0;
    }

    log_info("Opening acio device successful");

    uint8_t node_count = bt_acio_mgr_node_count_get(acio_manager_ctx);
    log_info("Enumerated %d nodes", node_count);

    kfca_node_id = -1;

    for (uint8_t i = 0; i < node_count; i++) {
        char product[4];
        bt_acio_mgr_node_product_ident_get(acio_manager_ctx, i, product);
        log_info(
            "> %d: %c%c%c%c\n",
            i,
            product[0],
            product[1],
            product[2],
            product[3]);

        if (!memcmp(product, "KFCA", 4)) {
            if (kfca_node_id != -1) {
                log_warning("Multiple KFCA found! Using highest node id.");
            }
            kfca_node_id = i;
        }
    }

    if (kfca_node_id != -1) {
        log_warning("Using KFCA on node: %d", kfca_node_id);

        uint8_t primary = 0;
        uint8_t headphone = 0;
        uint8_t subwoofer = 0;

        if (_sdvxio_kfca_config.override_main_volume >= 0) {
            primary = _sdvxio_kfca_config.override_main_volume;
        }
        if (_sdvxio_kfca_config.override_headphone_volume >= 0) {
            headphone = _sdvxio_kfca_config.override_headphone_volume;
        }
        if (_sdvxio_kfca_config.override_sub_volume >= 0) {
            subwoofer = _sdvxio_kfca_config.override_sub_volume;
        }

        bool init_result = aciodrv_kfca_amp(
            bt_acio_mgr_port_checkout(acio_manager_ctx),
            kfca_node_id,
            primary,
            headphone,
            0,
            subwoofer);
        bt_acio_mgr_port_checkin(acio_manager_ctx);

        if (!init_result) {
            log_warning("Unable to start KFCA on node: %d", kfca_node_id);
            return false;
        }

        running = true;
        log_warning("sdvxio-kfca now running");
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

    bt_acio_mgr_api_clear();
    module_acio_mgr_free(&acio_mgr_module);
}

void bt_io_sdvx_gpio_lights_set(uint32_t gpio_lights)
{
    pout_staging.gpio = gpio_lights;
    pout_staging.gpio |= 1 << 0x16;

    pout_staging.gpio = ac_io_u32(pout_staging.gpio);
}

void bt_io_sdvx_pwm_light_set(uint8_t light_no, uint8_t intensity)
{
    pout_staging.pwm[light_no] = intensity;
}

bool bt_io_sdvx_output_write(void)
{
    memcpy(&pout_ready, &pout_staging, sizeof(struct ac_io_kfca_poll_out));
    return true;
}

bool bt_io_sdvx_input_read(void)
{
    struct ac_io_kfca_poll_in pin;

    if (!running) {
        return false;
    }
    processing_io = true;

    bool poll_result = aciodrv_kfca_poll(
        bt_acio_mgr_port_checkout(acio_manager_ctx),
        kfca_node_id,
        &pout_ready,
        &pin);
    bt_acio_mgr_port_checkin(acio_manager_ctx);

    if (!poll_result) {
        return false;
    }

    processing_io = false;

    pin.adc[0] = ac_io_u16(pin.adc[0]);
    pin.adc[1] = ac_io_u16(pin.adc[1]);

    pin.gpio[0] = ac_io_u16(pin.gpio[0]);
    pin.gpio[1] = ac_io_u16(pin.gpio[1]);

    sdvx_io_gpio_sys = pin.gpio_sys;

    sdvx_io_analog[0] = (pin.adc[0] >> 6) & 0x3FF;
    sdvx_io_analog[1] = (pin.adc[1] >> 6) & 0x3FF;

    sdvx_io_gpio[0] = pin.gpio[0];
    sdvx_io_gpio[1] = pin.gpio[1];

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

    bool amp_result = aciodrv_kfca_amp(
        bt_acio_mgr_port_checkout(acio_manager_ctx),
        kfca_node_id,
        primary,
        headphone,
        96,
        subwoofer);
    bt_acio_mgr_port_checkin(acio_manager_ctx);

    if (!amp_result) {
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
    sdvxio_kfca_config_kfca_get(config, &_sdvxio_kfca_config);

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