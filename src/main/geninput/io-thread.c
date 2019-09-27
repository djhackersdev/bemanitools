#include <windows.h>

#include <stdlib.h>

#include "geninput/dev-list.h"
#include "geninput/hid.h"
#include "geninput/hid-generic.h"
#include "geninput/hid-mgr.h"
#include "geninput/io-thread.h"
#include "geninput/pacdrive.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/thread.h"

enum io_thread_cmd {
    IO_THREAD_CMD_STOP,
    IO_THREAD_CMD_ADD_DEVICE,
};

struct io_thread_msg {
    enum io_thread_cmd cmd;
    const char *dev_node;
    HANDLE barrier;
};

static HANDLE io_thread_cp;
static int io_thread_id;

static void io_thread_proc_add_device(const char *dev_node);
static void io_thread_proc_init(void);
static void io_thread_proc_main_loop(void);
static void io_thread_proc_fini(void);
static int io_thread_proc(void *param);

static void io_thread_proc_add_device(const char *dev_node)
{
    struct hid_stub *stub;
    struct hid_fd *hid_fd;
    uintptr_t ctx;

    hid_mgr_lock();

    stub = hid_mgr_get_named_stub(dev_node);

    if (!hid_stub_is_attached(stub)) {
        ctx = (uintptr_t) stub;

        if (pac_open(&hid_fd, dev_node, io_thread_cp, ctx)
                || hid_generic_open(&hid_fd, dev_node, io_thread_cp, ctx)) {
            hid_stub_attach(stub, (struct hid *) hid_fd);
        }
    }

    hid_mgr_unlock();
}

static void io_thread_proc_init(void)
{
    const char *dev_node;
    struct dev_list devs;

    io_thread_cp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    dev_list_init(&devs, &hid_guid);

    while (dev_list_next(&devs)) {
        dev_node = dev_list_get_dev_node(&devs);
        io_thread_proc_add_device(dev_node);
    }

    dev_list_fini(&devs);

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    log_info("USB I/O thread ready, thread id = %d",
            (unsigned int) GetCurrentThreadId());
}

static void io_thread_proc_main_loop(void)
{
    uintptr_t ctx;
    OVERLAPPED *p_ovl;
    DWORD nbytes;
    struct hid_stub *stub;
    struct io_thread_msg *msg;
    BOOL ok;

    for (;;) {
        ok = GetQueuedCompletionStatus(io_thread_cp, &nbytes,
                (ULONG_PTR *) &ctx, &p_ovl, 0);

        if (p_ovl != NULL) {
            /* An I/O completed (depending on the value of `ok' either
               successfuly or not), either way dispatch the completion event */

            stub = (struct hid_stub *) ctx;

            hid_mgr_lock();

            if (!ok) {
                log_warning("Async IO returned failure: %08x",
                        (unsigned int) GetLastError());
                hid_stub_detach(stub);
            } else {
                ok = hid_stub_handle_completion(stub, p_ovl, nbytes);
            }

            hid_mgr_unlock();

        } else if (ok) {
            /* No OVERLAPPED*, so this is a command message queued from a
               different thread. */

            msg = (struct io_thread_msg *) ctx;

            if (msg->cmd == IO_THREAD_CMD_STOP) {
                break;
            } else if (msg->cmd == IO_THREAD_CMD_ADD_DEVICE) {
                io_thread_proc_add_device(msg->dev_node);
                SetEvent(msg->barrier);
            }

        } else if (GetLastError() == WAIT_TIMEOUT) {
            /* Failure, but a benign one: timeout due to no event.
               I don't exactly know why, but we need to back off here for a bit
               or we end up soaking the CPU (in kernel mode...). It's probably
               a bemanitools bug instead of an NT deficiency but I'll have to
               track down the root cause later. */

            Sleep(1);
        } else {
            /* Otherwise idk what's going on */

            log_warning("Spurious wakeup in I/O mux: %08x",
                    (unsigned int) GetLastError());
        }
    }
}

static void io_thread_proc_fini(void)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    CloseHandle(io_thread_cp);

    log_misc("USB I/O thread shutting down");
}

static int io_thread_proc(void *param)
{
    HANDLE barrier;

    barrier = param;

    io_thread_proc_init();
    SetEvent(barrier);

    io_thread_proc_main_loop();

    io_thread_proc_fini();

    return 0;
}

void io_thread_init(void)
{
    HANDLE barrier;

    barrier = CreateEvent(NULL, TRUE, FALSE, NULL);

    io_thread_id = thread_create(io_thread_proc, barrier,
            16384, 0);

    WaitForSingleObject(barrier, INFINITE);
    CloseHandle(barrier);
}

void io_thread_add_device(const char *dev_node)
{
    struct io_thread_msg msg;

    msg.cmd = IO_THREAD_CMD_ADD_DEVICE;
    msg.dev_node = dev_node;
    msg.barrier = CreateEvent(NULL, TRUE, FALSE, NULL);

    PostQueuedCompletionStatus(io_thread_cp, 0, (uintptr_t) &msg, NULL);

    WaitForSingleObject(msg.barrier, INFINITE);
    CloseHandle(msg.barrier);
}

void io_thread_fini(void)
{
    struct io_thread_msg msg;

    msg.cmd = IO_THREAD_CMD_STOP;

    PostQueuedCompletionStatus(io_thread_cp, 0, (uintptr_t) &msg, NULL);

    thread_join(io_thread_id, NULL);
    thread_destroy(io_thread_id);
}

