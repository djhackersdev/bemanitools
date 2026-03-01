#define LOG_MODULE "ddrio-p4io"

#include <windows.h>

#include <stdatomic.h>
#include <stdlib.h>

#include "bemanitools/ddrio.h"
#include "bemanitools/input.h"

#include "cconfig/cconfig-main.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/time.h"

#include "config.h"

#include "aciodrv/device.h"
#include "aciodrv/mdxf.h"
#include "p4io/bitinfo.h"
#include "p4iodrv/device.h"

#include <stdio.h>

static struct p4iodrv_ctx *p4io_ctx;
static struct aciodrv_device_ctx *mdxf_device;

struct p4io_bittrans {
    uint32_t p4io;
    uint32_t ddrio;
};

static const struct p4io_bittrans input_map[] = {
    {(1 << 0), 1 << DDR_P1_START},
    {(1 << 1), 1 << DDR_P1_MENU_UP},
    {(1 << 2), 1 << DDR_P1_MENU_DOWN},
    {(1 << 3), 1 << DDR_P1_MENU_LEFT},
    {(1 << 4), 1 << DDR_P1_MENU_RIGHT},

    {(1 << 8), 1 << DDR_P2_START},
    {(1 << 9), 1 << DDR_P2_MENU_UP},
    {(1 << 10), 1 << DDR_P2_MENU_DOWN},
    {(1 << 11), 1 << DDR_P2_MENU_LEFT},
    {(1 << 12), 1 << DDR_P2_MENU_RIGHT},

    {(1 << 24), 1 << DDR_COIN},
    {(1 << 25), 1 << DDR_SERVICE},
    {(1 << 28), 1 << DDR_TEST},

};

// rotating r,g,b colors to cycle through for sd->p4io mapping.
static const uint8_t all_colors[][3] = {
    {0x00, 0x00, 0xFF},
    {0x00, 0xFF, 0x00},
    {0x00, 0xFF, 0xFF},
    {0xFF, 0x00, 0x00},
    {0xFF, 0x00, 0xFF},
    {0xFF, 0xFF, 0x00},
    {0xFF, 0xFF, 0xFF},
};
static const int num_of_colors = sizeof(all_colors) / sizeof(all_colors[0]);

uint8_t neon_index = 0;
uint8_t top_index = 0;
uint8_t bottom_index = 0;

bool prev_neon = false;
bool prev_top = false;
bool prev_bottom = false;

bool has_hdxs_lights = false;

p4io_lights_t light_buff = {0};
p4io_coinstock_t coin_buff = {0};
uint32_t jamma[4] = {0};

void ddr_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    log_to_external(misc, info, warning, fatal);
}

bool ddr_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    log_info("ddrio-p4io start");

    struct cconfig *config;
    struct p4io_mdxf_config config_mdxf;

    config = cconfig_init();
    ddrio_config_p4io_mdxf_init(config);

    if (!cconfig_main_config_init(
            config,
            "--ddrio-p4io-config",
            "ddrio-p4io.conf",
            "--help",
            "-h",
            "ddrio-p4io",
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    ddrio_config_p4io_mdxf_get(&config_mdxf, config);

    cconfig_finit(config);

    p4io_ctx = p4iodrv_open();

    if (!p4io_ctx) {
        log_warning("Could not open p4io");
        return false;
    }

    mdxf_device = aciodrv_device_open_path(config_mdxf.port, config_mdxf.baud);

    if (!mdxf_device) {
        log_warning("Opening acio device failed");
        return false;
    }

    int nodes = aciodrv_device_get_node_count(mdxf_device);

    if (nodes != 2) {
        log_warning("WARNING: only %d MDXF(s) found, expected 2", nodes);
    }

    for (int i = 0; i < nodes; i++) {
        if (!aciodrv_mdxf_init(mdxf_device, i)) {
            log_warning("Opening mdxf device %d failed", i);
            return false;
        }
    }

    return true;
}

uint32_t ddr_io_read_pad(void)
{
    uint32_t pad = 0;

    // pull the state of the p4io & get the state of the mdxfs
    if (p4io_ctx) {
        memset(jamma, 0, sizeof(jamma));

        if (p4iodrv_read_jamma(p4io_ctx, jamma)) {
            // map the p4io correctly.
            for (int i = 0; i < lengthof(input_map); i++) {
                // for ddrio, all of the inputs are in the first 32bit word of
                // jamma.
                if (jamma[0] & input_map[i].p4io) {
                    pad |= input_map[i].ddrio;
                }
            }
        } else {
            log_warning("Could not read opened p4io");
        }
    }

    struct ac_io_mdxf_poll_in foot_p1 = {0};
    struct ac_io_mdxf_poll_in foot_p2 = {0};

    // uint64_t start = time_get_counter();

    if (mdxf_device) {
        if (aciodrv_mdxf_poll(mdxf_device, 0, &foot_p1)) {
            pad |= (foot_p1.panel.up > 0) ? (1 << DDR_P1_UP) : 0;
            pad |= (foot_p1.panel.down > 0) ? (1 << DDR_P1_DOWN) : 0;
            pad |= (foot_p1.panel.left > 0) ? (1 << DDR_P1_LEFT) : 0;
            pad |= (foot_p1.panel.right > 0) ? (1 << DDR_P1_RIGHT) : 0;
        } else {
            log_warning("Could not read from opened p1 mdxf");
        }

        if (aciodrv_mdxf_poll(mdxf_device, 1, &foot_p2)) {
            pad |= (foot_p2.panel.up > 0) ? (1 << DDR_P2_UP) : 0;
            pad |= (foot_p2.panel.down > 0) ? (1 << DDR_P2_DOWN) : 0;
            pad |= (foot_p2.panel.left > 0) ? (1 << DDR_P2_LEFT) : 0;
            pad |= (foot_p2.panel.right > 0) ? (1 << DDR_P2_RIGHT) : 0;
        } else {
            log_warning("Could not read from opened p2 mdxf");
        }
    }

    // log_warning("poll time %lld", time_get_elapsed_us(time_get_counter() -
    // start));

    return pad;
}

void ddr_io_set_lights_extio(uint32_t lights)
{
    // Python4/white cab setup does not have pad lights. :(

    bool neon_on = (lights & (1 << LIGHT_NEONS));

    if (neon_on != prev_neon) {
        if (neon_on) {
            neon_index = (neon_index + 1) % num_of_colors;
        }

        light_buff.ddr.bass_r = neon_on ? all_colors[neon_index][0] : 0x00;
        light_buff.ddr.bass_g = neon_on ? all_colors[neon_index][1] : 0x00;
        light_buff.ddr.bass_b = neon_on ? all_colors[neon_index][2] : 0x00;
    }

    if (p4io_ctx) {
        p4iodrv_cmd_portout(p4io_ctx, (uint8_t *) &light_buff.raw);
    }

    prev_neon = neon_on;
}

void ddr_io_set_lights_p3io(uint32_t lights)
{
    bool top = (lights & (1 << LIGHT_P1_UPPER_LAMP)) ||
        (lights & (1 << LIGHT_P2_UPPER_LAMP));

    bool btm = (lights & (1 << LIGHT_P1_LOWER_LAMP)) ||
        (lights & (1 << LIGHT_P2_LOWER_LAMP));

    if (top != prev_top) {
        if (top) {
            top_index = (top_index + 1) % num_of_colors;
        }

        light_buff.ddr.header_up_r = top ? all_colors[top_index][0] : 0x00;
        light_buff.ddr.header_up_g = top ? all_colors[top_index][1] : 0x00;
        light_buff.ddr.header_up_b = top ? all_colors[top_index][2] : 0x00;
    }

    if (btm != prev_bottom) {
        if (btm) {
            bottom_index = (bottom_index + 1) % num_of_colors;
        }

        light_buff.ddr.header_down_r = btm ? all_colors[bottom_index][0] : 0x00;
        light_buff.ddr.header_down_g = btm ? all_colors[bottom_index][1] : 0x00;
        light_buff.ddr.header_down_b = btm ? all_colors[bottom_index][2] : 0x00;
    }

    if (!has_hdxs_lights) {
        // map the p3io sd lights -> white hd lights
        // if we aren't being given hdxs lights.
        light_buff.ddr.p1_start = (lights & (1 << LIGHT_P1_MENU)) ? 0xFF : 0x00;
        light_buff.ddr.p1_leftright =
            (lights & (1 << LIGHT_P1_MENU)) ? 0xFF : 0x00;

        // p2's lights are on the coinstock endpoint.
        coin_buff.ddr.p2_start = (lights & (1 << LIGHT_P1_MENU));
        coin_buff.ddr.p2_leftright = (lights & (1 << LIGHT_P1_MENU));

        if (p4io_ctx) {
            p4iodrv_cmd_coinstock(p4io_ctx, (uint8_t *) &coin_buff.raw);
        }
    }

    if (p4io_ctx) {
        p4iodrv_cmd_portout(p4io_ctx, (uint8_t *) &light_buff.raw);
    }

    prev_top = top;
    prev_bottom = btm;
}

void ddr_io_set_lights_hdxs_panel(uint32_t lights)
{
    if (!has_hdxs_lights) {
        log_warning("using hdxs lights for this power cycle");
        has_hdxs_lights = true;
    }

    light_buff.ddr.p1_start = (lights & (1 << LIGHT_HD_P1_START)) ? 0xFF : 0x00;
    light_buff.ddr.p1_updown =
        (lights & (1 << LIGHT_HD_P1_UP_DOWN)) ? 0xFF : 0x00;
    light_buff.ddr.p1_leftright =
        (lights & (1 << LIGHT_HD_P1_LEFT_RIGHT)) ? 0xFF : 0x00;

    // p2's lights are on the coinstock endpoint.
    coin_buff.ddr.p2_start = (lights & (1 << LIGHT_HD_P2_START));
    coin_buff.ddr.p2_leftright = (lights & (1 << LIGHT_HD_P2_LEFT_RIGHT));
    coin_buff.ddr.p2_updown = (lights & (1 << LIGHT_HD_P2_UP_DOWN));

    if (p4io_ctx) {
        p4iodrv_cmd_coinstock(p4io_ctx, (uint8_t *) &coin_buff.raw);
        p4iodrv_cmd_portout(p4io_ctx, (uint8_t *) &light_buff.raw);
    }
}

void ddr_io_set_lights_hdxs_rgb(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    // not supported.
}

void ddr_io_fini(void)
{
    if (mdxf_device) {
        aciodrv_device_close(mdxf_device);
    }

    if (p4io_ctx) {
        p4iodrv_close(p4io_ctx);
    }
}
