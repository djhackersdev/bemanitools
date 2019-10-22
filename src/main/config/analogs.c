// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <commctrl.h>
// clang-format on

#include <stdbool.h>
#include <stdlib.h>

#include "config/resource.h"
#include "config/schema.h"
#include "config/usages.h"

#include "geninput/hid-mgr.h"
#include "geninput/input-config.h"
#include "geninput/mapper.h"

#include "util/array.h"
#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"

#define SENSITIVITY_SCALE 4

struct analogs_ui {
    struct array children;
};

struct analog_ui {
    const struct analog_def *def;
    struct array hids;
    struct array control_nos;
    struct hid_stub *selected_hid;
    uint8_t pos;
};

static INT_PTR CALLBACK
analogs_ui_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static INT_PTR analogs_ui_handle_init(HWND hwnd, const PROPSHEETPAGE *psp);
static INT_PTR analogs_ui_handle_activate(HWND hwnd);
static INT_PTR analogs_ui_handle_passivate(HWND hwnd);
static INT_PTR analogs_ui_handle_tick(HWND hwnd);
static INT_PTR analogs_ui_handle_fini(HWND hwnd);

static INT_PTR CALLBACK
analog_ui_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static INT_PTR analog_ui_handle_init(HWND hwnd, struct analog_def *def);
static void analog_ui_handle_init_label(HWND hwnd);
static void analog_ui_handle_init_dev(HWND hwnd);
static void analog_ui_handle_init_sensitivity(HWND hwnd);
static bool analog_ui_match_device(struct hid_stub *hid);
static void analog_ui_populate_controls(HWND hwnd);
static INT_PTR analog_ui_handle_device_change(HWND hwnd);
static INT_PTR analog_ui_handle_control_change(HWND hwnd);
static INT_PTR analog_ui_handle_sensitivity_change(HWND hwnd);
static INT_PTR analog_ui_handle_tick(HWND hwnd);
static INT_PTR analog_ui_handle_fini(HWND hwnd);

HPROPSHEETPAGE
analogs_ui_tab_create(HINSTANCE inst, const struct schema *schema)
{
    PROPSHEETPAGE psp;

    memset(&psp, 0, sizeof(psp));
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = inst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_TAB_ANALOGS);
    psp.pfnDlgProc = analogs_ui_dlg_proc;
    psp.lParam = (LPARAM) schema;

    return CreatePropertySheetPage(&psp);
}

static INT_PTR CALLBACK
analogs_ui_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    const NMHDR *n;

    switch (msg) {
        case WM_INITDIALOG:
            return analogs_ui_handle_init(hwnd, (PROPSHEETPAGE *) lparam);

        case WM_NOTIFY:
            n = (NMHDR *) lparam;

            switch (n->code) {
                case PSN_SETACTIVE:
                    return analogs_ui_handle_activate(hwnd);

                case PSN_KILLACTIVE:
                    return analogs_ui_handle_passivate(hwnd);
            }

            return FALSE;

        case WM_TIMER:
            return analogs_ui_handle_tick(hwnd);

        case WM_DESTROY:
            return analogs_ui_handle_fini(hwnd);
    }

    return FALSE;
}

static INT_PTR analogs_ui_handle_init(HWND hwnd, const PROPSHEETPAGE *psp)
{
    struct analogs_ui *ui;
    const struct schema *schema;
    long ypos;
    size_t i;
    HINSTANCE inst;
    HWND child;
    RECT r;

    ui = xmalloc(sizeof(*ui));
    array_init(&ui->children);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) ui);

    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    schema = (struct schema *) psp->lParam;

    ypos = 0;

    for (i = 0; i < schema->nanalogs; i++) {
        child = CreateDialogParam(
            inst,
            MAKEINTRESOURCE(IDD_ANALOG),
            hwnd,
            analog_ui_dlg_proc,
            (LPARAM) &schema->analogs[i]);

        GetWindowRect(child, &r);
        SetWindowPos(
            child, HWND_BOTTOM, 0, ypos, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

        ypos += r.bottom - r.top;

        *array_append(HWND, &ui->children) = child;
    }

    return TRUE;
}

static INT_PTR analogs_ui_handle_activate(HWND hwnd)
{
    SetTimer(hwnd, 1, 17, NULL);

    return TRUE;
}

static INT_PTR analogs_ui_handle_passivate(HWND hwnd)
{
    KillTimer(hwnd, 1);

    return TRUE;
}

static INT_PTR analogs_ui_handle_tick(HWND hwnd)
{
    HWND child;
    struct analogs_ui *ui;
    size_t i;

    mapper_update();

    ui = (struct analogs_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    for (i = 0; i < ui->children.nitems; i++) {
        child = *array_item(HWND, &ui->children, i);
        SendMessage(child, WM_USER, 0, 0);
    }

    return TRUE;
}

static INT_PTR analogs_ui_handle_fini(HWND hwnd)
{
    struct analogs_ui *ui;

    ui = (struct analogs_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    array_fini(&ui->children);
    free(ui);

    return TRUE;
}

static INT_PTR CALLBACK
analog_ui_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_INITDIALOG:
            return analog_ui_handle_init(hwnd, (struct analog_def *) lparam);

        case WM_USER:
            return analog_ui_handle_tick(hwnd);

        case WM_COMMAND:
            switch (HIWORD(wparam)) {
                case CBN_SELCHANGE:
                    switch (LOWORD(wparam)) {
                        case IDC_DEVICE:
                            return analog_ui_handle_device_change(hwnd);

                        case IDC_CONTROL:
                            return analog_ui_handle_control_change(hwnd);

                        default:
                            return FALSE;
                    }

                default:
                    return FALSE;
            }

        case WM_HSCROLL:
            if (GetDlgItem(hwnd, IDC_SENSITIVITY) == (HWND) lparam) {
                return analog_ui_handle_sensitivity_change(hwnd);
            } else {
                return FALSE;
            }

        case WM_DESTROY:
            return analog_ui_handle_fini(hwnd);

        default:
            return FALSE;
    }
}

static INT_PTR analog_ui_handle_init(HWND hwnd, struct analog_def *def)
{
    struct analog_ui *ui;

    ui = xmalloc(sizeof(*ui));
    ui->def = def;
    array_init(&ui->hids);
    array_init(&ui->control_nos);
    ui->selected_hid = NULL;
    ui->pos = 0;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) ui);

    analog_ui_handle_init_label(hwnd);
    analog_ui_handle_init_dev(hwnd);
    analog_ui_handle_init_sensitivity(hwnd);

    return TRUE;
}

static void analog_ui_handle_init_label(HWND hwnd)
{
    struct analog_ui *ui;
    wchar_t label[128];
    HINSTANCE inst;
    HWND box;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    box = GetDlgItem(hwnd, IDC_GROUP);

    LoadString(inst, ui->def->label_rsrc, label, lengthof(label));
    SendMessage(box, WM_SETTEXT, 0, (LPARAM) label);
}

static void analog_ui_handle_init_dev(HWND hwnd)
{
    struct analog_ui *ui;
    struct mapped_analog ma;
    wchar_t *dev_name;
    struct hid_stub *hid;
    size_t nchars;
    LRESULT index;
    HWND dev_list;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    dev_list = GetDlgItem(hwnd, IDC_DEVICE);

    SendMessage(dev_list, CB_ADDSTRING, 0, (LPARAM) L"");

    hid_mgr_lock();

    ma.hid = NULL;
    mapper_get_analog_map(ui->def->tag, &ma);

    for (hid = hid_mgr_get_first_stub(); hid != NULL;
         hid = hid_mgr_get_next_stub(hid)) {
        if (!analog_ui_match_device(hid)) {
            continue;
        }

        if (!hid_stub_get_name(hid, NULL, &nchars)) {
            continue;
        }

        dev_name = xmalloc(nchars * sizeof(*dev_name));

        if (!hid_stub_get_name(hid, dev_name, &nchars)) {
            free(dev_name);

            continue;
        }

        index = SendMessage(dev_list, CB_ADDSTRING, 0, (LPARAM) dev_name);

        free(dev_name);

        *array_append(struct hid_stub *, &ui->hids) = hid;

        if (ma.hid == hid) {
            SendMessage(dev_list, CB_SETCURSEL, index, 0);

            ui->selected_hid = hid;
            analog_ui_populate_controls(hwnd);
        }
    }

    hid_mgr_unlock();
}

static void analog_ui_handle_init_sensitivity(HWND hwnd)
{
    struct analog_ui *ui;
    int pos;
    HWND slider;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    slider = GetDlgItem(hwnd, IDC_SENSITIVITY);
    pos = 256 * SENSITIVITY_SCALE + mapper_get_analog_sensitivity(ui->def->tag);

    SendMessage(slider, TBM_SETTICFREQ, 256, 0);
    SendMessage(
        slider,
        TBM_SETRANGE,
        FALSE,
        MAKELPARAM(0, SENSITIVITY_SCALE * 256 * 2));
    SendMessage(slider, TBM_SETPOS, TRUE, (LPARAM) pos);

    EnableWindow(slider, !mapper_is_analog_absolute(ui->def->tag));
}

static bool analog_ui_match_device(struct hid_stub *hid)
{
    struct hid_control *controls;
    size_t ncontrols;
    size_t i;

    if (!hid_stub_get_controls(hid, NULL, &ncontrols)) {
        goto size_fail;
    }

    controls = xmalloc(ncontrols * sizeof(*controls));

    if (!hid_stub_get_controls(hid, controls, &ncontrols)) {
        goto content_fail;
    }

    for (i = 0; i < ncontrols; i++) {
        if (controls[i].value_max - controls[i].value_min > 1) {
            break;
        }
    }

    free(controls);

    return i < ncontrols;

content_fail:
    free(controls);

size_fail:
    return false;
}

static void analog_ui_populate_controls(HWND hwnd)
{
    char usage_desc[512];
    wchar_t *tmp;
    struct analog_ui *ui;
    struct mapped_analog ma;
    struct hid_control *controls;
    size_t ncontrols;
    size_t i;
    long nitems;
    LRESULT index;
    HWND controls_ctl;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    controls_ctl = GetDlgItem(hwnd, IDC_CONTROL);
    nitems = (long) SendMessage(controls_ctl, CB_GETCOUNT, 0, 0);

    InvalidateRect(controls_ctl, NULL, TRUE);

    for (index = nitems; index >= 0; index--) {
        SendMessage(controls_ctl, CB_DELETESTRING, index, 0);
    }

    array_fini(&ui->control_nos);
    array_init(&ui->control_nos);

    if (ui->selected_hid == NULL) {
        return;
    }

    if (!hid_stub_get_controls(ui->selected_hid, NULL, &ncontrols)) {
        goto size_fail;
    }

    controls = xmalloc(sizeof(*controls) * ncontrols);

    if (!hid_stub_get_controls(ui->selected_hid, controls, &ncontrols)) {
        goto content_fail;
    }

    SendMessage(controls_ctl, CB_ADDSTRING, 0, (LPARAM) L"");

    mapper_get_analog_map(ui->def->tag, &ma);

    for (i = 0; i < ncontrols; i++) {
        if (controls[i].value_max - controls[i].value_min <= 1) {
            continue;
        }

        usages_get(usage_desc, lengthof(usage_desc), controls[i].usage);
        tmp = str_widen(usage_desc);

        index = SendMessage(controls_ctl, CB_ADDSTRING, 0, (LPARAM) tmp);
        free(tmp);

        if (i == ma.control_no) {
            SendMessage(controls_ctl, CB_SETCURSEL, index, 0);
        }

        *array_append(size_t, &ui->control_nos) = i;
    }

    free(controls);

    return;

content_fail:
    free(controls);

size_fail:
    return;
}

static INT_PTR analog_ui_handle_device_change(HWND hwnd)
{
    struct mapped_analog ma;
    struct analog_ui *ui;
    LRESULT index;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    index = SendMessage(GetDlgItem(hwnd, IDC_DEVICE), CB_GETCURSEL, 0, 0);

    log_assert((size_t) index <= ui->hids.nitems);

    if (index == 0) {
        ui->selected_hid = NULL;
    } else {
        ui->selected_hid = *array_item(struct hid_stub *, &ui->hids, index - 1);
    }

    ma.hid = NULL;
    mapper_set_analog_map(ui->def->tag, &ma);

    hid_mgr_lock();
    analog_ui_populate_controls(hwnd);
    hid_mgr_unlock();

    return TRUE;
}

static INT_PTR analog_ui_handle_control_change(HWND hwnd)
{
    struct mapped_analog ma;
    struct analog_ui *ui;
    LRESULT index;
    HWND slider;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    index = SendMessage(GetDlgItem(hwnd, IDC_CONTROL), CB_GETCURSEL, 0, 0);

    log_assert((size_t) index <= ui->control_nos.nitems);

    if (index == 0) {
        ma.hid = NULL;
    } else {
        ma.hid = ui->selected_hid;
        ma.control_no = *array_item(size_t, &ui->control_nos, index - 1);
    }

    mapper_set_analog_map(ui->def->tag, &ma);

    slider = GetDlgItem(hwnd, IDC_SENSITIVITY);

    EnableWindow(slider, !mapper_is_analog_absolute(ui->def->tag));

    return TRUE;
}

static INT_PTR analog_ui_handle_sensitivity_change(HWND hwnd)
{
    struct analog_ui *ui;
    int pos;
    HWND slider;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    slider = GetDlgItem(hwnd, IDC_SENSITIVITY);
    pos = (int) SendMessage(slider, TBM_GETPOS, 0, 0);

    mapper_set_analog_sensitivity(ui->def->tag, pos - 256 * SENSITIVITY_SCALE);

    return TRUE;
}

static INT_PTR analog_ui_handle_tick(HWND hwnd)
{
    struct analog_ui *ui;
    HWND pos_ctl;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    pos_ctl = GetDlgItem(hwnd, IDC_POSITION);

    ui->pos = mapper_read_analog(ui->def->tag);

    SendMessage(pos_ctl, WM_USER, 0, (LPARAM) ui->pos);

    return TRUE;
}

static INT_PTR analog_ui_handle_fini(HWND hwnd)
{
    struct analog_ui *ui;

    ui = (struct analog_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    array_fini(&ui->hids);
    array_fini(&ui->control_nos);
    free(ui);

    return TRUE;
}
