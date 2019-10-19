#include <wchar.h>
#include <windows.h>

#include "config/bind.h"
#include "config/resource.h"
#include "config/schema.h"
#include "config/snap.h"

#include "util/defs.h"

struct bind_state {
    const struct action_def *action;
    struct mapped_action *ma;
    struct snap snaps[2];
    size_t cur_snap;
    uintptr_t timer_id;
};

static INT_PTR CALLBACK
bind_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static INT_PTR bind_handle_init(HWND hwnd, struct bind_state *self);
static INT_PTR bind_handle_tick(HWND hwnd);
static INT_PTR bind_handle_cancel(HWND hwnd);
static INT_PTR bind_handle_destroy(HWND hwnd);

bool bind_control(
    HINSTANCE inst,
    HWND hwnd,
    const struct action_def *action,
    struct mapped_action *ma)
{
    struct bind_state self;
    bool result;

    self.action = action;
    self.ma = ma;
    self.cur_snap = 0;

    snap_init(&self.snaps[1]);
    snap_init(&self.snaps[0]);

    result = DialogBoxParam(
                 inst,
                 MAKEINTRESOURCE(IDD_BIND),
                 hwnd,
                 bind_dlg_proc,
                 (LPARAM) &self) != 0;

    snap_fini(&self.snaps[0]);
    snap_fini(&self.snaps[1]);

    return result;
}

static INT_PTR CALLBACK
bind_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_INITDIALOG:
            return bind_handle_init(hwnd, (struct bind_state *) lparam);

        case WM_TIMER:
            return bind_handle_tick(hwnd);

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDCANCEL:
                    return bind_handle_cancel(hwnd);

                default:
                    return FALSE;
            }

        case WM_DESTROY:
            return bind_handle_destroy(hwnd);

        default:
            return FALSE;
    }
}

static INT_PTR bind_handle_init(HWND hwnd, struct bind_state *self)
{
    HINSTANCE inst;
    wchar_t str[128];

    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    LoadString(inst, self->action->name_rsrc, str, lengthof(str));
    SetWindowText(GetDlgItem(hwnd, IDC_ACTION_NAME), str);

    self->timer_id = SetTimer(hwnd, 0, 17, NULL);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) self);

    return TRUE;
}

static INT_PTR bind_handle_tick(HWND hwnd)
{
    struct bind_state *self;

    self = (struct bind_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    self->cur_snap = !self->cur_snap;

    snap_fini(&self->snaps[self->cur_snap]);
    snap_init(&self->snaps[self->cur_snap]);

    if (snap_find_edge(
            &self->snaps[self->cur_snap],
            &self->snaps[!self->cur_snap],
            self->ma)) {
        EndDialog(hwnd, 1);
    }

    return TRUE;
}

static INT_PTR bind_handle_cancel(HWND hwnd)
{
    EndDialog(hwnd, 0);

    return TRUE;
}

static INT_PTR bind_handle_destroy(HWND hwnd)
{
    struct bind_state *self;

    self = (struct bind_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    KillTimer(hwnd, self->timer_id);

    return TRUE;
}
