// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <dbt.h>
// clang-format on

#include <stdint.h>

#include "geninput/hotplug.h"
#include "geninput/ri.h"

#include "util/log.h"
#include "util/msg-thread.h"

void msg_window_setup(HWND hwnd)
{
    ri_init(hwnd);
    hotplug_init(hwnd);

    log_info(
        "Message pump thread ready, thread id = %d",
        (unsigned int) GetCurrentThreadId());

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

LRESULT WINAPI
msg_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_INPUT:
            ri_handle_msg((HRAWINPUT) lparam);

            return 0;

        case WM_DEVICECHANGE:
            hotplug_handle_msg(wparam, (DEV_BROADCAST_HDR *) lparam);

            return TRUE;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

void msg_window_teardown(HWND hwnd)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

    log_info("Message pump thread shutting down");

    hotplug_fini();
    ri_fini();
}
