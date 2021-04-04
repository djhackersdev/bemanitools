// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
#include <stdlib.h>
// clang-format on

#include "aciomgr/manager.h"

#include "cconfig/cconfig-main.h"

#include "bemanitools/jbio.h"

#include "jbio-p4io/config-h44b.h"
#include "jbio-p4io/h44b.h"

#include "p4iodrv/device.h"

#include "util/log.h"

static struct p4iodrv_ctx *p4io_ctx;
static uint16_t jb_io_panels;
static uint8_t jb_io_sys_buttons;

static bool lights_present;
static struct ac_io_h44b_output jb_io_lights;
static struct ac_io_h44b_output jb_io_new_lights;

static bool coin_blocked;

void jb_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    aciomgr_set_loggers(misc, info, warning, fatal);

    log_to_external(misc, info, warning, fatal);
}

bool jb_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    struct cconfig *config;
    struct h44b_config config_h44b;

    config = cconfig_init();

    jbio_config_h44b_init(config);

    if (!cconfig_main_config_init(
            config,
            "--h44b-config",
            "jbio-h44b.conf",
            "--help",
            "-h",
            "jbio-h44b",
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    jbio_config_h44b_get(&config_h44b, config);

    cconfig_finit(config);

    p4io_ctx = p4iodrv_open();
    if(!p4io_ctx) {
        return false;
    }

    // some people use p4io just for inputs and have no lights. Soft fail when
    // h44b is not able to be connected instead of returning false
    lights_present = jb_io_h44b_init(config_h44b.port, config_h44b.baud);
    if(!lights_present) {
        log_warning("Could not connect to H44B, lights disabled");
    }

    return true;
}

void jb_io_fini(void)
{
    p4iodrv_close(p4io_ctx);
}

static const uint32_t jb_io_panel_mappings[] = {
    (1 << 5),
    (1 << 1),
    (1 << 13),
    (1 << 9),
    (1 << 6),
    (1 << 2),
    (1 << 14),
    (1 << 10),
    (1 << 7),
    (1 << 3),
    (1 << 15),
    (1 << 11),
    (1 << 16),
    (1 << 4),
    (1 << 20),
    (1 << 12),
};

static const uint32_t jb_io_sys_button_mappings[] = {
    (1 << 28),
    (1 << 25),
};

bool jb_io_read_inputs(void)
{
    uint32_t jamma[4];
    if(!p4iodrv_read_jamma(p4io_ctx, jamma)) {
        return false;
    }

    jb_io_panels = 0;
    jb_io_sys_buttons = 0;

    // panel is active low
    uint32_t panel_in = ~jamma[0];

    for (uint8_t i = 0; i < 16; i++) {
        if (panel_in & jb_io_panel_mappings[i]) {
            jb_io_panels |= 1 << i;
        }
    }

    // sys is active high
    for (uint8_t i = 0; i < 2; i++) {
        if (jamma[0] & jb_io_sys_button_mappings[i]) {
            jb_io_sys_buttons |= 1 << i;
        }
    }

    return true;
}

bool jb_io_write_lights(void)
{
    if(lights_present && memcmp(&jb_io_lights, &jb_io_new_lights, sizeof(jb_io_lights))) {
        memcpy(&jb_io_lights, &jb_io_new_lights, sizeof(jb_io_lights));
        return jb_io_h44b_write_lights(&jb_io_lights);
    }

    return true;
}

uint8_t jb_io_get_sys_inputs(void)
{
    return jb_io_sys_buttons;
}

uint16_t jb_io_get_panel_inputs(void)
{
    return jb_io_panels;
}

bool jb_io_set_panel_mode(enum jb_io_panel_mode mode)
{
    struct p4io_req_panel_mode panel_mode = {0};

    panel_mode.is_single = 1;

    switch(mode) {
        case JB_IO_PANEL_MODE_ALL:
            panel_mode.is_single = 0;
            panel_mode.mode = 0;
            break;
        case JB_IO_PANEL_MODE_TOP_LEFT:
            panel_mode.mode = 0x0001;
            break;
        case JB_IO_PANEL_MODE_TOP_RIGHT:
            panel_mode.mode = 0x0000;
            break;
        case JB_IO_PANEL_MODE_BOTTOM_LEFT:
            panel_mode.mode = 0x0101;
            break;
        case JB_IO_PANEL_MODE_BOTTOM_RIGHT:
            panel_mode.mode = 0x0100;
            break;
    }

    return p4iodrv_cmd_portout(p4io_ctx, (uint8_t*)&panel_mode);
}

void jb_io_set_rgb_led(enum jb_io_rgb_led unit, uint8_t r, uint8_t g, uint8_t b)
{
    // enum jb_io_rgb_led matches the ACIO message order
    uint8_t *raw_lights = (uint8_t*)&jb_io_new_lights;
    raw_lights[unit * 3] = r;
    raw_lights[unit * 3 + 1] = g;
    raw_lights[unit * 3 + 2] = b;
}

bool jb_io_set_coin_blocker(bool blocked) {
    coin_blocked = blocked;

    uint8_t coin[4] = {0};
    coin[0] = coin_blocked ? 0x00 : 0x20;
    return p4iodrv_cmd_coinstock(p4io_ctx, coin);
}
