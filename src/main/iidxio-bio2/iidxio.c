#include <windows.h>

#define LOG_MODULE "iidxio-bio2"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "bemanitools/glue.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-main.h"

#include "aciodrv/device.h"
#include "bio2drv/bi2a-iidx.h"
#include "bio2drv/config-bio2.h"
#include "bio2drv/detect.h"

#define log_misc(...) iidx_io_log_misc(LOG_MODULE, __VA_ARGS__)
#define log_info(...) iidx_io_log_info(LOG_MODULE, __VA_ARGS__)
#define log_warning(...) iidx_io_log_warning(LOG_MODULE, __VA_ARGS__)
#define log_fatal(...) iidx_io_log_fatal(LOG_MODULE, __VA_ARGS__)

static log_formatter_t iidx_io_log_misc;
static log_formatter_t iidx_io_log_info;
static log_formatter_t iidx_io_log_warning;
static log_formatter_t iidx_io_log_fatal;

static char autodetect_buffer[512];

static atomic_bool running;
static atomic_bool processing_io;
static int16_t bio2_node_id;

static struct bi2a_iidx_state_in pin_cur;
static struct bi2a_iidx_state_out pout_staging;
static struct bi2a_iidx_state_out pout_ready;

static struct aciodrv_device_ctx *bio2_device_ctx;

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

void iidx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    iidx_io_log_misc = misc;
    iidx_io_log_info = info;
    iidx_io_log_warning = warning;
    iidx_io_log_fatal = fatal;

    bio2drv_set_loggers(misc, info, warning, fatal);
}

bool iidx_io_init(
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

void iidx_io_fini(void)
{
    // Pushing some final state before closing the IO to the actual outputs, e.g. lights on/off
    // can be a bit finicky. Do a few polls to "enforce"/flush this final state 
    for (uint8_t i = 0; i < 3; i++) {
        iidx_io_ep1_send();
        iidx_io_ep2_recv();

        Sleep(10);
    }

    running = false;

    while (processing_io) {
        // avoid banging
        Sleep(1);
    }

    aciodrv_device_close();
}

void iidx_io_ep1_set_deck_lights(uint16_t deck_lights)
{
    for (uint8_t i = 0; i < sizeof(pout_staging.DECKSW); i++) {
        pout_staging.DECKSW[i].l_state = (deck_lights & (1 << i)) > 0;
    }    
}

void iidx_io_ep1_set_panel_lights(uint8_t panel_lights)
{
    pout_staging.PANEL[BI2A_IIDX_PANEL_BUTTON_START_P1].l_state = 
        (panel_lights & (1 << IIDX_IO_PANEL_P1_START)) > 0;
    pout_staging.PANEL[BI2A_IIDX_PANEL_BUTTON_START_P2].l_state = 
        (panel_lights & (1 << IIDX_IO_PANEL_P2_START)) > 0;
    pout_staging.PANEL[BI2A_IIDX_PANEL_BUTTON_VEFX].l_state = 
        (panel_lights & (1 << IIDX_IO_PANEL_VEFX)) > 0;
    pout_staging.PANEL[BI2A_IIDX_PANEL_BUTTON_EFFECT].l_state = 
        (panel_lights & (1 << IIDX_IO_PANEL_EFFECT)) > 0;
}

void iidx_io_ep1_set_top_lamps(uint8_t top_lamps)
{
    pout_staging.SPOTLIGHT1[BI2A_IIDX_SPOTLIGHT_RIGHT_RED].l_state = 
        (top_lamps & (1 << IIDX_IO_TOP_LAMP_RIGHT_RED)) > 0;
    pout_staging.SPOTLIGHT1[BI2A_IIDX_SPOTLIGHT_RIGHT_YELLOW].l_state = 
        (top_lamps & (1 << IIDX_IO_TOP_LAMP_RIGHT_YELLOW)) > 0;
    pout_staging.SPOTLIGHT1[BI2A_IIDX_SPOTLIGHT_RIGHT_GREEN].l_state = 
        (top_lamps & (1 << IIDX_IO_TOP_LAMP_RIGHT_GREEN)) > 0;
    pout_staging.SPOTLIGHT1[BI2A_IIDX_SPOTLIGHT_RIGHT_BLUE].l_state = 
        (top_lamps & (1 << IIDX_IO_TOP_LAMP_RIGHT_BLUE)) > 0;

    pout_staging.SPOTLIGHT2[BI2A_IIDX_SPOTLIGHT_LEFT_RED].l_state = 
        (top_lamps & (1 << IIDX_IO_TOP_LAMP_LEFT_RED)) > 0;
    pout_staging.SPOTLIGHT2[BI2A_IIDX_SPOTLIGHT_LEFT_YELLOW].l_state = 
        (top_lamps & (1 << IIDX_IO_TOP_LAMP_LEFT_YELLOW)) > 0;
    pout_staging.SPOTLIGHT2[BI2A_IIDX_SPOTLIGHT_LEFT_GREEN].l_state = 
        (top_lamps & (1 << IIDX_IO_TOP_LAMP_LEFT_GREEN)) > 0;
    pout_staging.SPOTLIGHT2[BI2A_IIDX_SPOTLIGHT_LEFT_BLUE].l_state = 
        (top_lamps & (1 << IIDX_IO_TOP_LAMP_LEFT_BLUE)) > 0;
}

void iidx_io_ep1_set_top_neons(bool top_neons)
{
    pout_staging.NEONLAMP.l_state = top_neons;
}

bool iidx_io_ep1_send(void)
{
    memcpy(&pout_ready, &pout_staging, sizeof(struct bi2a_iidx_state_out));

    return true;
}

bool iidx_io_ep2_recv(void)
{
    if (!_bio2_iidx_io_poll(&pout_ready, &pin_cur)) {
        return false;
    }

    return true;
}

uint8_t iidx_io_ep2_get_turntable(uint8_t player_no)
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

uint8_t iidx_io_ep2_get_slider(uint8_t slider_no)
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

uint8_t iidx_io_ep2_get_sys(void)
{
    uint8_t state;

    state = 0;

    if (pin_cur.SYSTEM.v_test) {
        state |= (1 << IIDX_IO_SYS_TEST);
    }

    if (pin_cur.SYSTEM.v_service) {
        state |= (1 << IIDX_IO_SYS_SERVICE);
    }

    if (pin_cur.SYSTEM.v_coin) {
        state |= (1 << IIDX_IO_SYS_COIN);
    }

    return state;
}

uint8_t iidx_io_ep2_get_panel(void)
{
    uint8_t state;

    state = 0;

    if (pin_cur.PANEL.y_start1) {
        state |= (1 << IIDX_IO_PANEL_P1_START);
    }

    if (pin_cur.PANEL.y_start2) {
        state |= (1 << IIDX_IO_PANEL_P2_START);
    }

    if (pin_cur.PANEL.y_vefx) {
        state |= (1 << IIDX_IO_PANEL_VEFX);
    }

    if (pin_cur.PANEL.y_effect) {
        state |= (1 << IIDX_IO_PANEL_EFFECT);
    }

    return state;
}

uint16_t iidx_io_ep2_get_keys(void)
{
    uint16_t state;

    state = 0;

    state |= ((pin_cur.P1SW1.b_val > 0) << IIDX_IO_KEY_P1_1);
    state |= ((pin_cur.P1SW2.b_val > 0) << IIDX_IO_KEY_P1_2);
    state |= ((pin_cur.P1SW3.b_val > 0) << IIDX_IO_KEY_P1_3);
    state |= ((pin_cur.P1SW4.b_val > 0) << IIDX_IO_KEY_P1_4);
    state |= ((pin_cur.P1SW5.b_val > 0) << IIDX_IO_KEY_P1_5);
    state |= ((pin_cur.P1SW6.b_val > 0) << IIDX_IO_KEY_P1_6);
    state |= ((pin_cur.P1SW7.b_val > 0) << IIDX_IO_KEY_P1_7);

    state |= ((pin_cur.P2SW1.b_val > 0) << IIDX_IO_KEY_P2_1);
    state |= ((pin_cur.P2SW2.b_val > 0) << IIDX_IO_KEY_P2_2);
    state |= ((pin_cur.P2SW3.b_val > 0) << IIDX_IO_KEY_P2_3);
    state |= ((pin_cur.P2SW4.b_val > 0) << IIDX_IO_KEY_P2_4);
    state |= ((pin_cur.P2SW5.b_val > 0) << IIDX_IO_KEY_P2_5);
    state |= ((pin_cur.P2SW6.b_val > 0) << IIDX_IO_KEY_P2_6);
    state |= ((pin_cur.P2SW7.b_val > 0) << IIDX_IO_KEY_P2_7);

    return state;
}

bool iidx_io_ep3_write_16seg(const char *text)
{
    memcpy(pout_staging.SEG16, text, sizeof(pout_staging.SEG16));

    return true;
}
