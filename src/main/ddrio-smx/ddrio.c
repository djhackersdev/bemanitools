#include <windows.h>

#include <stdatomic.h>
#include <stdlib.h>

#include "bemanitools/ddrio.h"
#include "bemanitools/input.h"

#include "core/log.h"

#include "imports/SMX.h"
#include "imports/avs.h"

#include "util/defs.h"

struct ddr_io_smx_pad_map {
    int pad_no;
    uint16_t smx_bit;
    uint32_t ddr_bit;
};

struct ddr_io_smx_light_map {
    uint32_t extio_bit;
    int smx_light_offset;
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

static void
ddr_io_smx_callback(int pad_no, enum SMXUpdateCallbackReason reason, void *ctx);

static const struct ddr_io_smx_pad_map ddr_io_smx_pad_map[] = {
    {0, 1 << 1, 1 << DDR_P1_UP},
    {0, 1 << 3, 1 << DDR_P1_LEFT},
    {0, 1 << 5, 1 << DDR_P1_RIGHT},
    {0, 1 << 7, 1 << DDR_P1_DOWN},

    {1, 1 << 1, 1 << DDR_P2_UP},
    {1, 1 << 3, 1 << DDR_P2_LEFT},
    {1, 1 << 5, 1 << DDR_P2_RIGHT},
    {1, 1 << 7, 1 << DDR_P2_DOWN},
};

#define DDR_IO_SMX_LIGHT_VALUES_PER_PANEL 75
#define DDR_IO_SMX_NUMBER_OF_PANELS 18
#define DDR_IO_SMX_TOTAL_LIGHT_VALUES \
    DDR_IO_SMX_LIGHT_VALUES_PER_PANEL *DDR_IO_SMX_NUMBER_OF_PANELS

static const struct ddr_io_smx_light_map ddr_io_smx_light_map[] = {
    /* Light L/R blue and U/D red to match DDR pad color scheme */

    {1 << LIGHT_P1_UP, DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 1, 0xFF, 0x00, 0x00},
    {1 << LIGHT_P1_LEFT,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 3,
     0x00,
     0x00,
     0xFF},
    {1 << LIGHT_P1_RIGHT,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 5,
     0x00,
     0x00,
     0xFF},
    {1 << LIGHT_P1_DOWN,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 7,
     0xFF,
     0x00,
     0x00},

    {1 << LIGHT_P2_UP,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 10,
     0xFF,
     0x00,
     0x00},
    {1 << LIGHT_P2_LEFT,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 12,
     0x00,
     0x00,
     0xFF},
    {1 << LIGHT_P2_RIGHT,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 14,
     0x00,
     0x00,
     0xFF},
    {1 << LIGHT_P2_DOWN,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 16,
     0xFF,
     0x00,
     0x00},
};

static _Atomic uint32_t ddr_io_smx_pad_state[2];
static CRITICAL_SECTION ddr_io_smx_lights_lock;
static uint8_t ddr_io_smx_lights_counter;
static char ddr_io_smx_lights[DDR_IO_SMX_TOTAL_LIGHT_VALUES];

void ddr_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    core_log_impl_set(misc, info, warning, fatal);
    input_set_loggers(misc, info, warning, fatal);

    /* We would need a log server thread to accept log messages from SMX, since
       it uses raw Win32 threads and not AVS threads (only AVS threads are
       permitted to use the AVS logging API). So it's not really worth it. */
}

bool ddr_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    /* Use geninput for menu/operator btns */

    input_init(thread_create, thread_join, thread_destroy);
    mapper_config_load("ddr");

    /* Start up SMX API */

    log_info("Starting SMX.DLL");
    log_info("SMX.DLL version: %s", "[predates SMX_Version()]");
    SMX_Start(ddr_io_smx_callback, NULL);
    log_info("Started SMX.DLL");

    InitializeCriticalSection(&ddr_io_smx_lights_lock);

    return true;
}

uint32_t ddr_io_read_pad(void)
{
    /* SMX pads require a constant stream of lighting updates or they will
       quickly revert to autonomous lighting control. Here is a hacky way of
       feeding that light data to the pad. Hopefully it won't starve anything
       important.

       Changing the light state causes the counter to reset, and the counter
       itself ought to roll over at a rate of about 4 Hz. */

    EnterCriticalSection(&ddr_io_smx_lights_lock);

    if (ddr_io_smx_lights_counter == 0) {
        SMX_SetLights2(ddr_io_smx_lights, lengthof(ddr_io_smx_lights));
    }

    ddr_io_smx_lights_counter++;

    LeaveCriticalSection(&ddr_io_smx_lights_lock);

    /* Sleep first: input is timestamped immediately AFTER the ioctl returns.

       Which is the right thing to do, for once. We sleep here because
       the game polls input in a tight loop. Can't complain, at there isn't
       an artificial limit on the poll frequency. */

    Sleep(1);

    /* We don't atomically read both pads, but they are separate USB devices
       so they don't update in lockstep anyway. */

    return mapper_update() | atomic_load(&ddr_io_smx_pad_state[0]) |
        atomic_load(&ddr_io_smx_pad_state[1]);
}

void ddr_io_set_lights_extio(uint32_t lights)
{
    const struct ddr_io_smx_light_map *map;
    size_t offset;
    size_t i;
    size_t j;

    EnterCriticalSection(&ddr_io_smx_lights_lock);

    ddr_io_smx_lights_counter = 0;
    memset(ddr_io_smx_lights, 0, sizeof(ddr_io_smx_lights));

    for (i = 0; i < lengthof(ddr_io_smx_light_map); i++) {
        map = &ddr_io_smx_light_map[i];

        if (lights & map->extio_bit) {
            offset = map->smx_light_offset;

            for (j = 0; j < DDR_IO_SMX_LIGHT_VALUES_PER_PANEL; j += 3) {
                ddr_io_smx_lights[offset + j] = map->r;
                ddr_io_smx_lights[offset + j + 1] = map->g;
                ddr_io_smx_lights[offset + j + 2] = map->b;
            }
        }
    }

    LeaveCriticalSection(&ddr_io_smx_lights_lock);
}

void ddr_io_set_lights_p3io(uint32_t lights)
{
    uint8_t i;

    for (i = 0x00; i <= 0x07; i++) {
        mapper_write_light(i, lights & (1 << i) ? 255 : 0);
    }
}

void ddr_io_set_lights_hdxs_panel(uint32_t lights)
{
    uint8_t i;

    for (i = 0x08; i <= 0x0D; i++) {
        mapper_write_light(i, lights & (1 << i) ? 255 : 0);
    }
}

void ddr_io_set_lights_hdxs_rgb(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx < 4) {
        uint8_t base = 0x20 + idx * 3;
        mapper_write_light(base + 0, r);
        mapper_write_light(base + 1, g);
        mapper_write_light(base + 2, b);
    }
}

void ddr_io_fini(void)
{
    log_info("Stopping SMX.DLL");
    SMX_Stop();
    log_info("Stopped SMX.DLL");

    DeleteCriticalSection(&ddr_io_smx_lights_lock);
    input_fini();
}

static void
ddr_io_smx_callback(int pad_no, enum SMXUpdateCallbackReason reason, void *ctx)
{
    const struct ddr_io_smx_pad_map *map;
    uint16_t smx_state;
    uint32_t ddr_state;
    size_t i;

    if (pad_no < 0 || pad_no > 1) {
        return;
    }

    smx_state = SMX_GetInputState(pad_no);
    ddr_state = 0;

    for (i = 0; i < lengthof(ddr_io_smx_pad_map); i++) {
        map = &ddr_io_smx_pad_map[i];

        if (pad_no == map->pad_no && (smx_state & map->smx_bit) != 0) {
            ddr_state |= map->ddr_bit;
        }
    }

    atomic_store(&ddr_io_smx_pad_state[pad_no], ddr_state);
}
