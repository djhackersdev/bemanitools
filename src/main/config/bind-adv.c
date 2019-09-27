#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config/bind-adv.h"
#include "config/resource.h"
#include "config/usages.h"

#include "geninput/hid-mgr.h"

#include "util/array.h"
#include "util/defs.h"
#include "util/mem.h"
#include "util/str.h"

struct bind_adv_state {
    uintptr_t timer_id;
    struct mapped_action ma;
    struct array devs;
    struct hid_stub *cur_hid;
    struct hid_control *ctls;
    size_t nctls;
    bool was_valid;
};

static INT_PTR CALLBACK bind_adv_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam,
        LPARAM lparam);
static INT_PTR bind_adv_handle_init(HWND hwnd, struct bind_adv_state *state);
static void bind_adv_handle_init_devs(HWND hwnd);
static void bind_adv_handle_init_insert_dev(HWND hwnd, struct hid_stub *hid);
static INT_PTR bind_adv_handle_change_ctl(HWND hwnd);
static INT_PTR bind_adv_handle_change_dev(HWND hwnd);
static INT_PTR bind_adv_handle_change_range(HWND hwnd);
static INT_PTR bind_adv_handle_tick(HWND hwnd);
static INT_PTR bind_adv_handle_ok(HWND hwnd);
static INT_PTR bind_adv_handle_fini(HWND hwnd);
static bool bind_adv_get_dev_no(HWND hwnd, size_t *dev_no_out);
static bool bind_adv_get_ctl_no(HWND hwnd, size_t *ctl_no_out);
static bool bind_adv_get_range(HWND hwnd, int32_t *out_min, int32_t *out_max);
static bool bind_adv_get_int(HWND control, int32_t *out);
static bool bind_adv_is_valid(HWND hwnd);
static void bind_adv_validate(HWND hwnd);
static void bind_adv_select_dev(HWND hwnd, size_t dev_no);
static void bind_adv_select_ctl(HWND hwnd, size_t ctl_no);
static void bind_adv_set_range(HWND hwnd, int32_t range_min, int32_t range_max);

bool bind_adv(HINSTANCE inst, HWND hwnd, struct mapped_action *ma,
        bool was_valid)
{
    struct bind_adv_state state;
    INT_PTR ok;

    memset(&state, 0, sizeof(state));

    state.ma = *ma;
    state.was_valid = was_valid;

    ok = DialogBoxParam(inst, MAKEINTRESOURCE(IDD_BIND_ADV), hwnd,
            bind_adv_dlg_proc, (LPARAM) &state) != 0;

    if (ok) {
        *ma = state.ma;
    }

    return ok != 0;
}

static INT_PTR CALLBACK bind_adv_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam,
        LPARAM lparam)
{
    switch (msg) {
        case WM_INITDIALOG:
            return bind_adv_handle_init(hwnd, (struct bind_adv_state *) lparam);

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDC_CONTROL:
                    switch (HIWORD(wparam)) {
                        case CBN_SELCHANGE:
                            return bind_adv_handle_change_ctl(hwnd);
                    }

                    return FALSE;

                case IDC_DEVICE:
                    switch (HIWORD(wparam)) {
                        case CBN_SELCHANGE:
                            return bind_adv_handle_change_dev(hwnd);
                    }

                    return FALSE;

                case IDC_BINDING_MIN:
                case IDC_BINDING_MAX:
                    switch (HIWORD(wparam)) {
                        case EN_CHANGE:
                            return bind_adv_handle_change_range(hwnd);
                    }

                    return FALSE;

                case IDOK:
                    switch (HIWORD(wparam)) {
                        case BN_CLICKED:
                            return bind_adv_handle_ok(hwnd);
                    }

                    return FALSE;

                case IDCANCEL:
                    switch (HIWORD(wparam)) {
                        case BN_CLICKED:
                            EndDialog(hwnd, FALSE);

                            return TRUE;
                    }

                    return FALSE;
            }

            return FALSE;

        case WM_TIMER:
            return bind_adv_handle_tick(hwnd);

        case WM_DESTROY:
            return bind_adv_handle_fini(hwnd);
    }

    return FALSE;
}

static INT_PTR bind_adv_handle_init(HWND hwnd, struct bind_adv_state *state)
{
    struct hid_stub *hid;
    size_t i;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) state);

    bind_adv_handle_init_devs(hwnd);

    state->timer_id = SetTimer(hwnd, 1, 17, NULL);

    if (!state->was_valid) {
        return TRUE;
    }

    for (i = 0 ; i < state->devs.nitems ; i++) {
        hid = *array_item(struct hid_stub *, &state->devs, i);

        if (hid == state->ma.hid) {
            bind_adv_select_dev(hwnd, (int) i);
            bind_adv_select_ctl(hwnd, state->ma.control_no);
            bind_adv_set_range(hwnd, state->ma.value_min,
                    state->ma.value_max);
        }
    }

    return TRUE;
}

static void bind_adv_handle_init_devs(HWND hwnd)
{
    struct hid_stub *hid;

    hid_mgr_lock();

    for (hid = hid_mgr_get_first_stub()
            ; hid != NULL
            ; hid = hid_mgr_get_next_stub(hid)) {
        bind_adv_handle_init_insert_dev(hwnd, hid);
    }

    hid_mgr_unlock();
}

static void bind_adv_handle_init_insert_dev(HWND hwnd, struct hid_stub *hid)
{
    struct bind_adv_state *state;
    wchar_t *name;
    size_t nchars;
    HWND devs;

    state = (struct bind_adv_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    devs = GetDlgItem(hwnd, IDC_DEVICE);

    if (!hid_stub_get_name(hid, NULL, &nchars)) {
        goto size_fail;
    }

    name = xmalloc(sizeof(*name) * nchars);

    if (!hid_stub_get_name(hid, name, &nchars)) {
        goto name_fail;
    }

    *array_append(struct hid_stub *, &state->devs) = hid;

    SendMessage(devs, CB_ADDSTRING, 0, (LPARAM) name);

name_fail:
    free(name);

size_fail:
    ;
}

static INT_PTR bind_adv_handle_change_ctl(HWND hwnd)
{
    size_t ctl_no;

    if (bind_adv_get_ctl_no(hwnd, &ctl_no)) {
        bind_adv_select_ctl(hwnd, ctl_no);
    }

    return TRUE;
}

static INT_PTR bind_adv_handle_change_dev(HWND hwnd)
{
    size_t dev_no;

    if (bind_adv_get_dev_no(hwnd, &dev_no)) {
        bind_adv_select_dev(hwnd, dev_no);
    }

    return TRUE;
}

static INT_PTR bind_adv_handle_change_range(HWND hwnd)
{
    bind_adv_validate(hwnd);

    return TRUE;
}

static INT_PTR bind_adv_handle_tick(HWND hwnd)
{
    struct bind_adv_state *state;
    wchar_t wchars[16];
    size_t ctl_no;
    int32_t value;
    HWND label;

    state = (struct bind_adv_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (!bind_adv_get_ctl_no(hwnd, &ctl_no)) {
        return TRUE;
    }

    hid_mgr_lock();

    if (hid_stub_get_value(state->cur_hid, ctl_no, &value)) {
        wstr_format(wchars, lengthof(wchars), L"%d", value);

        label = GetDlgItem(hwnd, IDC_CURRENT);
        SetWindowText(label, wchars);
    }

    hid_mgr_unlock();

    return TRUE;
}

static INT_PTR bind_adv_handle_ok(HWND hwnd)
{
    struct bind_adv_state *state;
    size_t dev_no;
    size_t ctl_no;

    state = (struct bind_adv_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    /* bind_adv_validate() ought to ensure that OK is disabled whenever the
       user's input is somehow invalid. Still, it never hurts to double check.

       Ignore any OK clicks that we somehow receive with invalid input.  */

    if (!bind_adv_get_dev_no(hwnd, &dev_no)) {
        return TRUE;
    }

    state->ma.hid = *array_item(struct hid_stub *, &state->devs, dev_no);

    if (!bind_adv_get_ctl_no(hwnd, &ctl_no)) {
        return TRUE;
    }

    state->ma.control_no = ctl_no;

    if (!bind_adv_get_range(hwnd, &state->ma.value_min, &state->ma.value_max)) {
        return TRUE;
    }

    /* Input OK, shut down this dialog. */

    EndDialog(hwnd, TRUE);

    return TRUE;
}

static INT_PTR bind_adv_handle_fini(HWND hwnd)
{
    struct bind_adv_state *state;

    state = (struct bind_adv_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    array_fini(&state->devs);

    KillTimer(hwnd, state->timer_id);

    return TRUE;
}

static bool bind_adv_get_dev_no(HWND hwnd, size_t *dev_no_out)
{
    const struct bind_adv_state *state;
    size_t dev_no;
    HWND devs;

    state = (struct bind_adv_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    devs = GetDlgItem(hwnd, IDC_DEVICE);

    dev_no = SendMessage(devs, CB_GETCURSEL, 0, 0);

    if (dev_no >= state->devs.nitems) {
        return false;
    }

    if (dev_no_out != NULL) {
        *dev_no_out = dev_no;
    }

    return true;
}

static bool bind_adv_get_ctl_no(HWND hwnd, size_t *ctl_no_out)
{
    const struct bind_adv_state *state;
    size_t ctl_no;
    HWND ctls;

    state = (struct bind_adv_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ctls = GetDlgItem(hwnd, IDC_CONTROL);

    ctl_no = SendMessage(ctls, CB_GETCURSEL, 0, 0);

    if (ctl_no >= state->nctls) {
        return false;
    }

    if (ctl_no_out != NULL) {
        *ctl_no_out = ctl_no;
    }

    return true;
}

static bool bind_adv_get_range(HWND hwnd, int32_t *out_min, int32_t *out_max)
{
    HWND wnd_min;
    HWND wnd_max;

    wnd_min = GetDlgItem(hwnd, IDC_BINDING_MIN);
    wnd_max = GetDlgItem(hwnd, IDC_BINDING_MAX);

    return bind_adv_get_int(wnd_min, out_min)
            && bind_adv_get_int(wnd_max, out_max);
}

static bool bind_adv_get_int(HWND control, int32_t *out)
{
    wchar_t text[16];
    int32_t tmp;

    text[0] = L'\0';
    GetWindowText(control, text, lengthof(text));

    if (swscanf(text, L"%d", &tmp) != 1) {
        return false;
    }

    if (out != NULL) {
        *out = tmp;
    }

    return true;
}

static bool bind_adv_is_valid(HWND hwnd)
{
    if (!bind_adv_get_dev_no(hwnd, NULL)) {
        return false;
    }

    if (!bind_adv_get_ctl_no(hwnd, NULL)) {
        return false;
    }

    if (!bind_adv_get_range(hwnd, NULL, NULL)) {
        return false;
    }

    return true;
}

static void bind_adv_validate(HWND hwnd)
{
    HWND ok;

    ok = GetDlgItem(hwnd, IDOK);
    EnableWindow(ok, bind_adv_is_valid(hwnd) ? TRUE : FALSE);
}

static void bind_adv_select_dev(HWND hwnd, size_t dev_no)
{
    struct bind_adv_state *state;
    struct hid_stub *hid;
    char chars[256];
    wchar_t wchars[256];
    size_t i;
    HWND ctls;
    HWND devs;

    state = (struct bind_adv_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ctls = GetDlgItem(hwnd, IDC_CONTROL);
    devs = GetDlgItem(hwnd, IDC_DEVICE);

    if (dev_no >= state->devs.nitems) {
        return;
    }

    SendMessage(devs, CB_SETCURSEL, (WPARAM) dev_no, 0);
    SendMessage(ctls, CB_RESETCONTENT, 0, 0);

    hid = *array_item(struct hid_stub *, &state->devs, dev_no);

    state->cur_hid = hid;

    hid_mgr_lock();

    if (!hid_stub_get_controls(state->cur_hid, NULL, &state->nctls)) {
        goto size_fail;
    }

    free(state->ctls);
    state->ctls = xmalloc(sizeof(*state->ctls) * state->nctls);

    if (!hid_stub_get_controls(state->cur_hid, state->ctls, &state->nctls)) {
        goto data_fail;
    }

    for (i = 0 ; i < state->nctls ; i++) {
        wchars[0] = L'\0';

        usages_get(chars, lengthof(chars), state->ctls[i].usage);
        MultiByteToWideChar(CP_UTF8, 0, chars, -1, wchars, lengthof(wchars));

        SendMessage(ctls, CB_ADDSTRING, 0, (LPARAM) wchars);
    }

    hid_mgr_unlock();

    bind_adv_validate(hwnd);

    return;

data_fail:
    free(state->ctls);

    state->ctls = NULL;
    state->nctls = 0;

size_fail:
    hid_mgr_unlock();

    bind_adv_validate(hwnd);
}

static void bind_adv_select_ctl(HWND hwnd, size_t ctl_no)
{
    wchar_t text[128];
    struct bind_adv_state *state;
    const struct hid_control *ctl;
    HWND ctls;
    HWND limit_min;
    HWND limit_max;

    state = (struct bind_adv_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ctls = GetDlgItem(hwnd, IDC_CONTROL);
    limit_min = GetDlgItem(hwnd, IDC_LIMIT_MIN);
    limit_max = GetDlgItem(hwnd, IDC_LIMIT_MAX);

    if (ctl_no >= state->nctls) {
        return;
    }

    SendMessage(ctls, CB_SETCURSEL, (WPARAM) ctl_no, 0);

    ctl = &state->ctls[ctl_no];

    wstr_format(text, lengthof(text), L"%d", ctl->value_min);
    SetWindowText(limit_min, text);

    wstr_format(text, lengthof(text), L"%d", ctl->value_max);
    SetWindowText(limit_max, text);

    bind_adv_validate(hwnd);
}

static void bind_adv_set_range(HWND hwnd, int32_t range_min, int32_t range_max)
{
    wchar_t text[16];
    HWND wnd_min;
    HWND wnd_max;

    wnd_min = GetDlgItem(hwnd, IDC_BINDING_MIN);
    wnd_max = GetDlgItem(hwnd, IDC_BINDING_MAX);

    wstr_format(text, lengthof(text), L"%d", range_min);
    SetWindowText(wnd_min, text);

    wstr_format(text, lengthof(text), L"%d", range_max);
    SetWindowText(wnd_max, text);

    bind_adv_validate(hwnd);
}

