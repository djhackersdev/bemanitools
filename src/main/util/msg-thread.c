#define LOG_MODULE "msg-thread"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "util/log.h"
#include "util/msg-thread.h"
#include "util/thread.h"

static bool msg_thread_step(HWND hwnd);

static const char msg_wndclass_name[] = "msg-thread";

static int msg_thread_id;
static HANDLE msg_thread_ready;
static HANDLE msg_thread_stop;
static HWND msg_window;

static int msg_thread_proc(void *param)
{
    HWND hwnd;
    HINSTANCE inst;
    MSG msg;
    WNDCLASSEX wcx;

    inst = param;

    memset(&wcx, 0, sizeof(wcx));

    wcx.cbSize = sizeof(wcx);
    wcx.lpfnWndProc = msg_window_proc; /* callback */
    wcx.hInstance = inst;
    wcx.lpszClassName = msg_wndclass_name;

    RegisterClassEx(&wcx);

    /* Message-only windows (children of HWND_MESSAGE) do not receive
       broadcast events.

       Because THAT totally makes sense. */

    hwnd = CreateWindowEx(
        0, msg_wndclass_name, NULL, 0, 0, 0, 0, 0, NULL, NULL, inst, NULL);

    msg_window_setup(hwnd); /* callback */

    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    SetEvent(msg_thread_ready);

    while (msg_thread_step(hwnd))
        ;

    msg_window_teardown(hwnd); /* callback */

    DestroyWindow(hwnd);
    UnregisterClass(msg_wndclass_name, inst);

    return 0;
}

static bool msg_thread_step(HWND hwnd)
{
    MSG msg;
    DWORD result;

    result = MsgWaitForMultipleObjectsEx(
        1, &msg_thread_stop, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);

    switch (result) {
        case WAIT_OBJECT_0:
            return false;

        case WAIT_OBJECT_0 + 1:
            while (PeekMessage(&msg, msg_window, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            return true;

        case WAIT_IO_COMPLETION:
            return true;

        case WAIT_FAILED:
            log_warning(
                "MsgWaitForMultipleObjectsEx failed: %08x",
                (unsigned int) GetLastError());

            return false;

        default:
            log_warning(
                "Spurious wakeup: result = %08x", (unsigned int) result);

            return true;
    }
}

void msg_thread_init(HINSTANCE inst)
{
    msg_thread_ready = CreateEvent(NULL, TRUE, FALSE, NULL);
    msg_thread_stop = CreateEvent(NULL, TRUE, FALSE, NULL);
    msg_thread_id = thread_create(msg_thread_proc, inst, 0x4000, 0);

    WaitForSingleObject(msg_thread_ready, INFINITE);
    CloseHandle(msg_thread_ready);
}

void msg_thread_fini(void)
{
    SetEvent(msg_thread_stop);

    thread_join(msg_thread_id, NULL);
    thread_destroy(msg_thread_id);

    CloseHandle(msg_thread_stop);
}
