#define LOG_MODULE "ddrio-smx"

#include <windows.h>

#include <stdatomic.h>
#include <stdlib.h>

#include "api/core/log.h"
#include "api/core/thread.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface/input.h"

#include "imports/SMX.h"
#include "imports/avs.h"

#include "main/module/input-ext.h"
#include "main/module/input.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/input.h"
#include "sdk/module/io/ddr.h"

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
    {0, 1 << 1, 1 << BT_IO_DDR_P1_UP},
    {0, 1 << 3, 1 << BT_IO_DDR_P1_LEFT},
    {0, 1 << 5, 1 << BT_IO_DDR_P1_RIGHT},
    {0, 1 << 7, 1 << BT_IO_DDR_P1_DOWN},

    {1, 1 << 1, 1 << BT_IO_DDR_P2_UP},
    {1, 1 << 3, 1 << BT_IO_DDR_P2_LEFT},
    {1, 1 << 5, 1 << BT_IO_DDR_P2_RIGHT},
    {1, 1 << 7, 1 << BT_IO_DDR_P2_DOWN},
};

#define DDR_IO_SMX_LIGHT_VALUES_PER_PANEL 75
#define DDR_IO_SMX_NUMBER_OF_PANELS 18
#define DDR_IO_SMX_TOTAL_LIGHT_VALUES \
    DDR_IO_SMX_LIGHT_VALUES_PER_PANEL *DDR_IO_SMX_NUMBER_OF_PANELS

static const struct ddr_io_smx_light_map ddr_io_smx_light_map[] = {
    /* Light L/R blue and U/D red to match DDR pad color scheme */

    {1 << BT_IO_DDR_EXTIO_LIGHT_P1_UP,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 1,
     0xFF,
     0x00,
     0x00},
    {1 << BT_IO_DDR_EXTIO_LIGHT_P1_LEFT,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 3,
     0x00,
     0x00,
     0xFF},
    {1 << BT_IO_DDR_EXTIO_LIGHT_P1_RIGHT,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 5,
     0x00,
     0x00,
     0xFF},
    {1 << BT_IO_DDR_EXTIO_LIGHT_P1_DOWN,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 7,
     0xFF,
     0x00,
     0x00},

    {1 << BT_IO_DDR_EXTIO_LIGHT_P2_UP,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 10,
     0xFF,
     0x00,
     0x00},
    {1 << BT_IO_DDR_EXTIO_LIGHT_P2_LEFT,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 12,
     0x00,
     0x00,
     0xFF},
    {1 << BT_IO_DDR_EXTIO_LIGHT_P2_RIGHT,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 14,
     0x00,
     0x00,
     0xFF},
    {1 << BT_IO_DDR_EXTIO_LIGHT_P2_DOWN,
     DDR_IO_SMX_LIGHT_VALUES_PER_PANEL * 16,
     0xFF,
     0x00,
     0x00},
};

static _Atomic uint32_t ddr_io_smx_pad_state[2];
static CRITICAL_SECTION ddr_io_smx_lights_lock;
static uint8_t ddr_io_smx_lights_counter;
static char ddr_io_smx_lights[DDR_IO_SMX_TOTAL_LIGHT_VALUES];
static module_input_t *_ddr_io_smx_module_input;

bool bt_io_ddr_init()
{
    bool result;
    bt_input_api_t input_api;

    /* Use geninput for menu/operator btns */
    module_input_ext_load_and_init("geninput.dll", &_ddr_io_smx_module_input);
    module_input_api_get(_ddr_io_smx_module_input, &input_api);
    bt_input_api_set(&input_api);

    result = bt_input_mapper_config_load("ddr");

    if (!result) {
        log_warning("Failed loading input mapper config for ddr");
        return false;
    }

    /* Start up SMX API */

    log_info("Starting SMX.DLL");
    log_info("SMX.DLL version: %s", "[predates SMX_Version()]");
    SMX_Start(ddr_io_smx_callback, NULL);
    log_info("Started SMX.DLL");

    InitializeCriticalSection(&ddr_io_smx_lights_lock);

    return true;
}

uint32_t bt_io_ddr_pad_read(void)
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

    return bt_input_mapper_update() | atomic_load(&ddr_io_smx_pad_state[0]) |
        atomic_load(&ddr_io_smx_pad_state[1]);
}

void bt_io_ddr_extio_lights_set(uint32_t lights)
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

void bt_io_ddr_p3io_lights_set(uint32_t lights)
{
    uint8_t i;

    for (i = 0x00; i <= 0x07; i++) {
        bt_input_mapper_light_write(i, lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_ddr_hdxs_lights_panel_set(uint32_t lights)
{
    uint8_t i;

    for (i = 0x08; i <= 0x0D; i++) {
        bt_input_mapper_light_write(i, lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_ddr_hdxs_lights_rgb_set(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx < 4) {
        uint8_t base = 0x20 + idx * 3;
        bt_input_mapper_light_write(base + 0, r);
        bt_input_mapper_light_write(base + 1, g);
        bt_input_mapper_light_write(base + 2, b);
    }
}

void bt_io_ddr_fini(void)
{
    log_info("Stopping SMX.DLL");
    SMX_Stop();
    log_info("Stopped SMX.DLL");

    DeleteCriticalSection(&ddr_io_smx_lights_lock);

    bt_input_fini();

    bt_input_api_clear();
    module_input_free(&_ddr_io_smx_module_input);
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

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);

    /* We would need a log server thread to accept log messages from SMX, since
       it uses raw Win32 threads and not AVS threads (only AVS threads are
       permitted to use the AVS logging API). So it's not really worth it. */
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_ddr_api_get(bt_io_ddr_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_ddr_init;
    api->v1.fini = bt_io_ddr_fini;
    api->v1.pad_read = bt_io_ddr_pad_read;
    api->v1.extio_lights_set = bt_io_ddr_extio_lights_set;
    api->v1.p3io_lights_set = bt_io_ddr_p3io_lights_set;
    api->v1.hdxs_lights_panel_set = bt_io_ddr_hdxs_lights_panel_set;
    api->v1.hdxs_lights_rgb_set = bt_io_ddr_hdxs_lights_rgb_set;
}