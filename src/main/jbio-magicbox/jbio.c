// This implementation is ported from the device.dll source graciously
// provided by zyp

#include <windows.h>

#include "bemanitools/jbio.h"
#include "imports/ch341.h"
#include "util/defs.h"

static uint16_t jb_io_panels;
static uint8_t jb_io_sys_buttons;

static bool is_initialized = false;

static log_formatter_t jb_io_log_misc;
static log_formatter_t jb_io_log_info;
static log_formatter_t jb_io_log_warning;
static log_formatter_t jb_io_log_fatal;

union magicbox_input {
    uint32_t dword;
    uint8_t bytes[4];
};

void jb_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    jb_io_log_misc = misc;
    jb_io_log_info = info;
    jb_io_log_warning = warning;
    jb_io_log_fatal = fatal;
}

bool jb_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    if (CH341OpenDevice(0) < 0) {
        jb_io_log_warning("jbio", "Can't open CH341 device.\n");
        return false;
    }

    is_initialized = true;

    return true;
}

void jb_io_fini(void)
{
    CH341CloseDevice(0);
}

static const uint32_t magic_panel_mappings[] = {
    (1 << 0x08),
    (1 << 0x0a),
    (1 << 0x0c),
    (1 << 0x0e),
    (1 << 0x07),
    (1 << 0x05),
    (1 << 0x03),
    (1 << 0x01),
    (1 << 0x09),
    (1 << 0x0b),
    (1 << 0x0d),
    (1 << 0x0f),
    (1 << 0x06),
    (1 << 0x04),
    (1 << 0x02),
    (1 << 0x00),
};

static const uint32_t magic_sys_mappings[] = {
    (1 << 0x11), // TEST
    (1 << 0x10), // SERVICE
    (1 << 0x13), // COIN
};

bool jb_io_read_inputs(void)
{
    // Read IO board
    unsigned long size;
    union magicbox_input input;

    input.dword = -1;
    jb_io_panels = 0;
    jb_io_sys_buttons = 0;

    CH341EppSetAddr(0, 255);
    CH341EppSetAddr(0, 1);
    CH341EppSetAddr(0, 255);
    CH341EppSetAddr(0, 0);
    size = sizeof(uint8_t);
    CH341EppReadData(0, &input.bytes[0], &size);
    CH341EppSetAddr(0, 255);
    CH341EppSetAddr(0, 0);
    size = sizeof(uint8_t);
    CH341EppReadData(0, &input.bytes[1], &size);
    CH341EppSetAddr(0, 255);
    CH341EppSetAddr(0, 0);
    size = sizeof(uint8_t);
    CH341EppReadData(0, &input.bytes[2], &size);
    CH341EppSetAddr(0, 255);

    for (uint8_t i = 0; i < lengthof(magic_panel_mappings); i++) {
        if ((input.dword & magic_panel_mappings[i]) == 0) {
            jb_io_panels |= 1 << i;
        }
    }
    for (uint8_t i = 0; i < lengthof(magic_sys_mappings); i++) {
        if ((input.dword & magic_sys_mappings[i]) == 0) {
            jb_io_sys_buttons |= 1 << i;
        }
    }

    return true;
}

bool jb_io_write_outputs(void)
{
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

void jb_io_set_rgb_led(enum jb_io_rgb_led unit, uint8_t r, uint8_t g, uint8_t b)
{
    // I mean I guess there's reactive LEDs on the sides? I'm not going to the
    // effort to work out if they're controllable or not
}
