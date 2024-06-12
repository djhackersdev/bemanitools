#define LOG_MODULE "aciodrv-proc-panb"

#include <string.h>

#include "aciodrv/device.h"
#include "aciodrv/panb.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

static int auto_poll_proc(void *auto_poll_param);
static int auto_poll_threadid;

static CRITICAL_SECTION keypair_lock;
static struct ac_io_panb_keypair _keypair[AC_IO_PANB_MAX_KEYPAIRS];

static CRITICAL_SECTION auto_poll_stop_lock;
static bool auto_poll_stop;

static int auto_poll_proc(void *param)
{
    struct aciodrv_device_ctx *device = (struct aciodrv_device_ctx *) param;
    struct ac_io_panb_poll_in poll_in;
    bool stop;

    do {
        aciodrv_panb_recv_poll(device, &poll_in);

        EnterCriticalSection(&keypair_lock);
        memcpy(_keypair, poll_in.keypair, AC_IO_PANB_MAX_KEYPAIRS);
        LeaveCriticalSection(&keypair_lock);

        EnterCriticalSection(&auto_poll_stop_lock);
        stop = auto_poll_stop;
        LeaveCriticalSection(&auto_poll_stop_lock);
    } while (!stop);

    return 0;
}

bool aciodrv_proc_panb_init(struct aciodrv_device_ctx *device)
{
    bt_core_thread_result_t result;

    log_assert(device);

    if (!aciodrv_panb_start_auto_input(device, 0, AC_IO_PANB_NUM_NODES)) {
        return false;
    }

    auto_poll_stop = false;
    InitializeCriticalSection(&keypair_lock);
    InitializeCriticalSection(&auto_poll_stop_lock);

    result = bt_core_thread_create(
        auto_poll_proc, (void *) device, 0x4000, 0, &auto_poll_threadid);
    bt_core_thread_fatal_on_error(result);

    return true;
}

bool aciodrv_proc_panb_get_state(uint8_t *button_state)
{
    struct ac_io_panb_keypair keypair[AC_IO_PANB_MAX_KEYPAIRS];

    EnterCriticalSection(&keypair_lock);
    memcpy(keypair, _keypair, AC_IO_PANB_MAX_KEYPAIRS);
    LeaveCriticalSection(&keypair_lock);

    /* splice the keypairs into separate button values */
    for (int i = 0; i < AC_IO_PANB_MAX_KEYPAIRS; i++) {
        uint8_t but1 = keypair[i].key1;
        uint8_t but2 = keypair[i].key2;
        button_state[2 * i] = but1;
        button_state[2 * i + 1] = but2;
    }

    return true;
}

void aciodrv_proc_panb_fini(struct aciodrv_device_ctx *device)
{
    bt_core_thread_result_t result;

    EnterCriticalSection(&auto_poll_stop_lock);
    auto_poll_stop = true;
    LeaveCriticalSection(&auto_poll_stop_lock);

    result = bt_core_thread_join(auto_poll_threadid, NULL);
    bt_core_thread_fatal_on_error(result);

    result = bt_core_thread_destroy(auto_poll_threadid);
    bt_core_thread_fatal_on_error(result);

    DeleteCriticalSection(&keypair_lock);
    DeleteCriticalSection(&auto_poll_stop_lock);

    /* reset is the only way to disable the auto polling on device side */
    aciodrv_device_reset(device);
}