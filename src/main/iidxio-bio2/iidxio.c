#define LOG_MODULE "iidxio-bio2"

#include <windows.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "aciodrv/device.h"

#include "api/core/log.h"
#include "api/core/thread.h"

#include "bio2drv/bi2a-iidx.h"
#include "bio2drv/config-bio2.h"
#include "bio2drv/detect.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "cconfig/cconfig-main.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/io/iidx.h"

static char autodetect_buffer[512];

static atomic_bool running;
static atomic_bool processing_io;
static int16_t bio2_node_id;

static struct bi2a_iidx_state_in pin_cur;
static struct bi2a_iidx_state_out pout_staging;
static struct bi2a_iidx_state_out pout_ready;

static struct aciodrv_device_ctx *bio2_device_ctx;

bool bt_io_iidx_ep1_send(void);
bool bt_io_iidx_ep2_recv(void);

static bool _bio2_iidx_io_poll(
    const struct bi2a_iidx_state_out *pout, struct bi2a_iidx_state_in *pin)
{
    if (!running) {
        return false;
    }

    processing_io = true;

    if (!bio2drv_bi2a_iidx_poll(bio2_device_ctx, bio2_node_id, pout, pin)) {
        processing_io = false;
        return false;
    }

    processing_io = false;
    return true;
}

bool bt_io_iidx_init()
{
    struct cconfig *config;
    struct bio2drv_config_bio2 config_bio2;

    config = cconfig_init();

    bio2drv_config_bio2_init(config);

    if (!cconfig_main_config_init(
            config,
            "--bio2-config",
            "iidxio-bio2.conf",
            "--help",
            "-h",
            "iidxio-bio2",
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

    // BIO2's cannot share a bus with anything else, so use device directly
    bio2_device_ctx = aciodrv_device_open_path(selected_port, config_bio2.baud);

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

        if (!bio2drv_bi2a_iidx_init(bio2_device_ctx, bio2_node_id)) {
            log_warning("Unable to start BI2A on node: %d", bio2_node_id);
            return false;
        }

        running = true;
        log_warning("iidxio-bio2 now running");
    } else {
        log_warning("No bio2 device found");
    }

    return running;
}

void bt_io_iidx_fini(void)
{
    // Pushing some final state before closing the IO to the actual outputs,
    // e.g. lights on/off can be a bit finicky. Do a few polls to
    // "enforce"/flush this final state
    for (uint8_t i = 0; i < 3; i++) {
        bt_io_iidx_ep1_send();
        bt_io_iidx_ep2_recv();

        Sleep(10);
    }

    running = false;

    while (processing_io) {
        // avoid banging
        Sleep(1);
    }

    aciodrv_device_close(bio2_device_ctx);
}

void bt_io_iidx_ep1_deck_lights_set(uint16_t deck_lights)
{
    for (uint8_t i = 0; i < sizeof(pout_staging.DECKSW); i++) {
        pout_staging.DECKSW[i].l_state = (deck_lights & (1 << i)) > 0;
    }
}

void bt_io_iidx_ep1_panel_lights_set(uint8_t panel_lights)
{
    pout_staging.PANEL[BI2A_IIDX_PANEL_BUTTON_START_P1].l_state =
        (panel_lights & (1 << BT_IO_IIDX_PANEL_P1_START)) > 0;
    pout_staging.PANEL[BI2A_IIDX_PANEL_BUTTON_START_P2].l_state =
        (panel_lights & (1 << BT_IO_IIDX_PANEL_P2_START)) > 0;
    pout_staging.PANEL[BI2A_IIDX_PANEL_BUTTON_VEFX].l_state =
        (panel_lights & (1 << BT_IO_IIDX_PANEL_VEFX)) > 0;
    pout_staging.PANEL[BI2A_IIDX_PANEL_BUTTON_EFFECT].l_state =
        (panel_lights & (1 << BT_IO_IIDX_PANEL_EFFECT)) > 0;
}

void bt_io_iidx_ep1_top_lamps_set(uint8_t top_lamps)
{
    pout_staging.SPOTLIGHT1[BI2A_IIDX_SPOTLIGHT_RIGHT_RED].l_state =
        (top_lamps & (1 << BT_IO_IIDX_TOP_LAMP_RIGHT_RED)) > 0;
    pout_staging.SPOTLIGHT1[BI2A_IIDX_SPOTLIGHT_RIGHT_YELLOW].l_state =
        (top_lamps & (1 << BT_IO_IIDX_TOP_LAMP_RIGHT_YELLOW)) > 0;
    pout_staging.SPOTLIGHT1[BI2A_IIDX_SPOTLIGHT_RIGHT_GREEN].l_state =
        (top_lamps & (1 << BT_IO_IIDX_TOP_LAMP_RIGHT_GREEN)) > 0;
    pout_staging.SPOTLIGHT1[BI2A_IIDX_SPOTLIGHT_RIGHT_BLUE].l_state =
        (top_lamps & (1 << BT_IO_IIDX_TOP_LAMP_RIGHT_BLUE)) > 0;

    pout_staging.SPOTLIGHT2[BI2A_IIDX_SPOTLIGHT_LEFT_RED].l_state =
        (top_lamps & (1 << BT_IO_IIDX_TOP_LAMP_LEFT_RED)) > 0;
    pout_staging.SPOTLIGHT2[BI2A_IIDX_SPOTLIGHT_LEFT_YELLOW].l_state =
        (top_lamps & (1 << BT_IO_IIDX_TOP_LAMP_LEFT_YELLOW)) > 0;
    pout_staging.SPOTLIGHT2[BI2A_IIDX_SPOTLIGHT_LEFT_GREEN].l_state =
        (top_lamps & (1 << BT_IO_IIDX_TOP_LAMP_LEFT_GREEN)) > 0;
    pout_staging.SPOTLIGHT2[BI2A_IIDX_SPOTLIGHT_LEFT_BLUE].l_state =
        (top_lamps & (1 << BT_IO_IIDX_TOP_LAMP_LEFT_BLUE)) > 0;
}

void bt_io_iidx_ep1_top_neons_set(bool top_neons)
{
    pout_staging.NEONLAMP.l_state = top_neons;
}

bool bt_io_iidx_ep1_send(void)
{
    memcpy(&pout_ready, &pout_staging, sizeof(struct bi2a_iidx_state_out));

    return true;
}

bool bt_io_iidx_ep2_recv(void)
{
    if (!_bio2_iidx_io_poll(&pout_ready, &pin_cur)) {
        return false;
    }

    return true;
}

uint8_t bt_io_iidx_ep2_turntable_get(uint8_t player_no)
{
    switch (player_no) {
        case 0:
            return pin_cur.TURNTABLE1;
        case 1:
            return pin_cur.TURNTABLE2;
        default:
            return 0;
    }
}

uint8_t bt_io_iidx_ep2_slider_get(uint8_t slider_no)
{
    switch (slider_no) {
        case 0:
            return pin_cur.SLIDER1.s_val & 0xF;
        case 1:
            return pin_cur.SLIDER2.s_val & 0xF;
        case 2:
            return pin_cur.SLIDER3.s_val & 0xF;
        case 3:
            return pin_cur.SLIDER4.s_val & 0xF;
        case 4:
            return pin_cur.SLIDER5.s_val & 0xF;
        default:
            return 0;
    }
}

uint8_t bt_io_iidx_ep2_sys_get(void)
{
    uint8_t state;

    state = 0;

    if (pin_cur.SYSTEM.v_test) {
        state |= (1 << BT_IO_IIDX_SYS_TEST);
    }

    if (pin_cur.SYSTEM.v_service) {
        state |= (1 << BT_IO_IIDX_SYS_SERVICE);
    }

    if (pin_cur.SYSTEM.v_coin) {
        state |= (1 << BT_IO_IIDX_SYS_COIN);
    }

    return state;
}

uint8_t bt_io_iidx_ep2_panel_get(void)
{
    uint8_t state;

    state = 0;

    if (pin_cur.PANEL.y_start1) {
        state |= (1 << BT_IO_IIDX_PANEL_P1_START);
    }

    if (pin_cur.PANEL.y_start2) {
        state |= (1 << BT_IO_IIDX_PANEL_P2_START);
    }

    if (pin_cur.PANEL.y_vefx) {
        state |= (1 << BT_IO_IIDX_PANEL_VEFX);
    }

    if (pin_cur.PANEL.y_effect) {
        state |= (1 << BT_IO_IIDX_PANEL_EFFECT);
    }

    return state;
}

uint16_t bt_io_iidx_ep2_keys_get(void)
{
    uint16_t state;

    state = 0;

    state |= ((pin_cur.P1SW1.b_val > 0) << BT_IO_IIDX_KEY_P1_1);
    state |= ((pin_cur.P1SW2.b_val > 0) << BT_IO_IIDX_KEY_P1_2);
    state |= ((pin_cur.P1SW3.b_val > 0) << BT_IO_IIDX_KEY_P1_3);
    state |= ((pin_cur.P1SW4.b_val > 0) << BT_IO_IIDX_KEY_P1_4);
    state |= ((pin_cur.P1SW5.b_val > 0) << BT_IO_IIDX_KEY_P1_5);
    state |= ((pin_cur.P1SW6.b_val > 0) << BT_IO_IIDX_KEY_P1_6);
    state |= ((pin_cur.P1SW7.b_val > 0) << BT_IO_IIDX_KEY_P1_7);

    state |= ((pin_cur.P2SW1.b_val > 0) << BT_IO_IIDX_KEY_P2_1);
    state |= ((pin_cur.P2SW2.b_val > 0) << BT_IO_IIDX_KEY_P2_2);
    state |= ((pin_cur.P2SW3.b_val > 0) << BT_IO_IIDX_KEY_P2_3);
    state |= ((pin_cur.P2SW4.b_val > 0) << BT_IO_IIDX_KEY_P2_4);
    state |= ((pin_cur.P2SW5.b_val > 0) << BT_IO_IIDX_KEY_P2_5);
    state |= ((pin_cur.P2SW6.b_val > 0) << BT_IO_IIDX_KEY_P2_6);
    state |= ((pin_cur.P2SW7.b_val > 0) << BT_IO_IIDX_KEY_P2_7);

    return state;
}

bool bt_io_iidx_ep3_16seg_send(const char *text)
{
    memcpy(pout_staging.SEG16, text, sizeof(pout_staging.SEG16));

    return true;
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_iidx_api_get(bt_io_iidx_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_iidx_init;
    api->v1.fini = bt_io_iidx_fini;
    api->v1.ep1_deck_lights_set = bt_io_iidx_ep1_deck_lights_set;
    api->v1.ep1_panel_lights_set = bt_io_iidx_ep1_panel_lights_set;
    api->v1.ep1_top_lamps_set = bt_io_iidx_ep1_top_lamps_set;
    api->v1.ep1_top_neons_set = bt_io_iidx_ep1_top_neons_set;
    api->v1.ep1_send = bt_io_iidx_ep1_send;
    api->v1.ep2_recv = bt_io_iidx_ep2_recv;
    api->v1.ep2_turntable_get = bt_io_iidx_ep2_turntable_get;
    api->v1.ep2_slider_get = bt_io_iidx_ep2_slider_get;
    api->v1.ep2_sys_get = bt_io_iidx_ep2_sys_get;
    api->v1.ep2_panel_get = bt_io_iidx_ep2_panel_get;
    api->v1.ep2_keys_get = bt_io_iidx_ep2_keys_get;
    api->v1.ep3_16seg_send = bt_io_iidx_ep3_16seg_send;
}
