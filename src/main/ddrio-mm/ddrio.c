#include <windows.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "bemanitools/ddrio.h"

#include "mm/mm.h"

#include "util/cmdline.h"
#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"

struct ddr_bittrans {
    uint32_t mm;
    uint32_t p3io;
};

static const struct ddr_bittrans input_map[] = {
    {0x00000001, 1 << DDR_SERVICE},
    {0x00000002, 1 << DDR_TEST},
    {0x00100000, 1 << DDR_P1_MENU_LEFT},
    {0x00400000, 1 << DDR_P1_MENU_RIGHT},
    {0x00000100, 1 << DDR_P1_START},
    {0x00200000, 1 << DDR_P2_MENU_LEFT},
    {0x00800000, 1 << DDR_P2_MENU_RIGHT},
    {0x00000200, 1 << DDR_P2_START},
    {0x00004000, 1 << DDR_P1_LEFT},
    {0x00010000, 1 << DDR_P1_RIGHT},
    {0x00000400, 1 << DDR_P1_UP},
    {0x00001000, 1 << DDR_P1_DOWN},
    {0x00008000, 1 << DDR_P2_LEFT},
    {0x00020000, 1 << DDR_P2_RIGHT},
    {0x00000800, 1 << DDR_P2_UP},
    {0x00002000, 1 << DDR_P2_DOWN},

    /* Nonstandard */
    {0x01000000, 1 << DDR_P1_MENU_UP},
    {0x04000000, 1 << DDR_P1_MENU_DOWN},
    {0x02000000, 1 << DDR_P2_MENU_UP},
    {0x08000000, 1 << DDR_P2_MENU_DOWN},
};

static const struct ddr_bittrans extio_light_map[] = {
    {0x00000100, 1 << LIGHT_P1_UP},
    {0x00000200, 1 << LIGHT_P1_DOWN},
    {0x00000400, 1 << LIGHT_P1_LEFT},
    {0x00000800, 1 << LIGHT_P1_RIGHT},
    {0x00010000, 1 << LIGHT_P2_UP},
    {0x00020000, 1 << LIGHT_P2_DOWN},
    {0x00040000, 1 << LIGHT_P2_LEFT},
    {0x00080000, 1 << LIGHT_P2_RIGHT},
    {0x01000000, 1 << LIGHT_NEONS},
};

static const struct ddr_bittrans p3io_light_map[] = {
    {0x00000004, 1 << LIGHT_P1_MENU},
    {0x00000008, 1 << LIGHT_P2_MENU},
    {0x00000010, 1 << LIGHT_P2_LOWER_LAMP},
    {0x00000020, 1 << LIGHT_P2_UPPER_LAMP},
    {0x00000040, 1 << LIGHT_P1_LOWER_LAMP},
    {0x00000080, 1 << LIGHT_P1_UPPER_LAMP},
};

static bool initted;
static CRITICAL_SECTION cs;
static struct mm_output out;

static int ddr_io_get_lag_param(void)
{
    int argc;
    char **argv;
    int result;
    int i;

    result = 0;

    args_recover(&argc, &argv);

    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            continue;
        }

        switch (argv[i][1]) {
            case 'a':
                if (i + 1 == argc) {
                    continue;
                }

                result = atoi(argv[i + 1]);

                break;
        }
    }

    args_free(argc, argv);

    if (result < 0) {
        /* snark snark */
        log_warning(
            "This PCB is incapable of seeing into the future. "
            "Defaulting to 0 injected lag samples");

        result = 0;
    }

    return result;
}

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
    bool ok;

    log_assert(!initted);

    ok = mm_init(2 + ddr_io_get_lag_param());

    if (!ok) {
        return false;
    }

    InitializeCriticalSection(&cs);
    out.lights = 0x00101000; /* Hold pad IO board FSMs in reset */

    initted = true;

    return true;
}

uint32_t ddr_io_read_pad(void)
{
    struct mm_input in;
    uint32_t i;
    uint32_t pad;

    EnterCriticalSection(&cs);

    mm_update(&out, &in);
    pad = 0;

    for (i = 0; i < lengthof(input_map); i++) {
        if (in.jamma & input_map[i].mm) {
            pad |= input_map[i].p3io;
        }
    }

    LeaveCriticalSection(&cs);

    return pad;
}

void ddr_io_set_lights_extio(uint32_t extio_lights)
{
    uint32_t clr;
    uint32_t set;
    int i;

    clr = 0;
    set = 0;

    for (i = 0; i < lengthof(extio_light_map); i++) {
        if (extio_lights & extio_light_map[i].p3io /* misnomer but w/e */) {
            set |= extio_light_map[i].mm;
        } else {
            clr |= extio_light_map[i].mm;
        }
    }

    atomic_fetch_or(&out.lights, set);
    atomic_fetch_and(&out.lights, ~clr);
}

void ddr_io_set_lights_p3io(uint32_t p3io_lights)
{
    uint32_t clr;
    uint32_t set;
    int i;

    clr = 0;
    set = 0;

    for (i = 0; i < lengthof(p3io_light_map); i++) {
        if (p3io_lights & p3io_light_map[i].p3io) {
            set |= p3io_light_map[i].mm;
        } else {
            clr |= p3io_light_map[i].mm;
        }
    }

    atomic_fetch_or(&out.lights, set);
    atomic_fetch_and(&out.lights, ~clr);
}

void ddr_io_set_lights_hdxs_panel(uint32_t lights)
{
    (void) lights;
    // stubbed
}

void ddr_io_set_lights_hdxs_rgb(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{

    (void) idx;
    (void) r;
    (void) g;
    (void) b;
    // stubbed
}

void ddr_io_fini(void)
{
    if (initted) {
        mm_fini();
        DeleteCriticalSection(&cs);
        initted = false;
    }
}
