// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
#include <windowsx.h>
// clang-format on

#include <stdint.h>
#include <stdlib.h>

#include "config/bind-light.h"
#include "config/resource.h"
#include "config/schema.h"
#include "config/usages.h"

#include "geninput/input-config.h"
#include "geninput/mapper.h"

#include "util/array.h"
#include "util/defs.h"
#include "util/mem.h"

#define PULSE_DELTA 0.03f

struct lights_ui {
    const struct schema *schema;
    struct array devices;
    struct hid_stub *hid;
    struct hid_light *lights;
    size_t nlights;
    bool pulse_active;
    bool pulse_up;
    size_t pulse_light_no;
    float pulse_coeff;
};

static INT_PTR CALLBACK
lights_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static INT_PTR lights_handle_init(HWND hwnd, const PROPSHEETPAGE *psp);
static void lights_handle_init_devices(HWND hwnd);
static void lights_handle_init_device(HWND hwnd, struct hid_stub *hid);
static void lights_handle_init_lights(HWND hwnd);
static INT_PTR lights_handle_change_dev(HWND hwnd);
static INT_PTR lights_handle_highlight_light(HWND hwnd, const NMLISTVIEW *n);
static INT_PTR lights_handle_bind_light(HWND hwnd);
static INT_PTR lights_handle_pulse_tick(HWND hwnd);
static void lights_update_bindings(HWND hwnd);
static void lights_pulse_start(HWND hwnd, size_t light_no);
static void lights_pulse_stop(HWND hwnd);
static INT_PTR lights_handle_fini(HWND hwnd);

HPROPSHEETPAGE
lights_tab_create(HINSTANCE inst, const struct schema *schema)
{
    PROPSHEETPAGE psp;

    memset(&psp, 0, sizeof(psp));
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = inst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_TAB_LIGHTS);
    psp.pfnDlgProc = lights_dlg_proc;
    psp.lParam = (LPARAM) schema;

    return CreatePropertySheetPage(&psp);
}

static INT_PTR CALLBACK
lights_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    NMHDR *n;

    switch (msg) {
        case WM_INITDIALOG:
            return lights_handle_init(hwnd, (const PROPSHEETPAGE *) lparam);

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDC_DEVICE:
                    switch (HIWORD(wparam)) {
                        case CBN_SELCHANGE:
                            return lights_handle_change_dev(hwnd);
                    }

                    return FALSE;
            }

            return FALSE;

        case WM_NOTIFY:
            n = (NMHDR *) lparam;

            switch (n->idFrom) {
                case IDC_LIGHT:
                    switch (n->code) {
                        case LVN_ITEMCHANGED:
                            return lights_handle_highlight_light(
                                hwnd, (NMLISTVIEW *) n);

                        case NM_DBLCLK:
                            return lights_handle_bind_light(hwnd);
                    }

                    return FALSE;
            }

            return FALSE;

        case WM_TIMER:
            return lights_handle_pulse_tick(hwnd);

        case WM_DESTROY:
            return lights_handle_fini(hwnd);
    }

    return FALSE;
}

static INT_PTR lights_handle_init(HWND hwnd, const PROPSHEETPAGE *psp)
{
    struct lights_ui *ui;

    ui = xmalloc(sizeof(*ui));
    ui->schema = (const struct schema *) psp->lParam;
    ui->hid = NULL;
    ui->lights = NULL;
    array_init(&ui->devices);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (INT_PTR) ui);

    lights_handle_init_devices(hwnd);
    lights_handle_init_lights(hwnd);

    return TRUE;
}

static void lights_handle_init_devices(HWND hwnd)
{
    struct hid_stub *hid;

    hid_mgr_lock();

    for (hid = hid_mgr_get_first_stub(); hid != NULL;
         hid = hid_mgr_get_next_stub(hid)) {
        lights_handle_init_device(hwnd, hid);
    }

    hid_mgr_unlock();
}

static void lights_handle_init_device(HWND hwnd, struct hid_stub *hid)
{
    wchar_t *chars;
    size_t nchars;
    size_t nlights;
    struct lights_ui *ui;
    HWND devs;

    if (!hid_stub_get_lights(hid, NULL, &nlights) || nlights == 0) {
        goto lights_fail;
    }

    if (!hid_stub_get_name(hid, NULL, &nchars)) {
        goto name_sz_fail;
    }

    chars = xmalloc(sizeof(*chars) * nchars);

    if (!hid_stub_get_name(hid, chars, &nchars)) {
        goto name_fail;
    }

    ui = (struct lights_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    devs = GetDlgItem(hwnd, IDC_DEVICE);

    *array_append(struct hid_stub *, &ui->devices) = hid;
    ComboBox_AddString(devs, chars);

    return;

name_fail:
    free(chars);

name_sz_fail:
lights_fail:
    return;
}

static void lights_handle_init_lights(HWND hwnd)
{
    wchar_t str[128];
    HINSTANCE inst;
    LVCOLUMN col;
    HWND lights;

    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    lights = GetDlgItem(hwnd, IDC_LIGHT);

    ListView_SetExtendedListViewStyle(
        lights, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LoadString(inst, IDS_COL_LIGHT_HID, str, lengthof(str));

    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    col.fmt = LVCFMT_LEFT;
    col.cx = 120;
    col.pszText = str;

    ListView_InsertColumn(lights, 0, &col);

    LoadString(inst, IDS_COL_LIGHT_GAME, str, lengthof(str));

    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    col.fmt = LVCFMT_LEFT;
    col.cx = 120;
    col.pszText = str;

    ListView_InsertColumn(lights, 1, &col);
}

static INT_PTR lights_handle_change_dev(HWND hwnd)
{
    char chars[256];
    wchar_t wchars[256];
    struct lights_ui *ui;
    struct hid_stub *hid;
    size_t i;
    int dev_idx;
    LVITEM item;
    HWND devs_ctl;
    HWND lights_ctl;

    lights_pulse_stop(hwnd);

    ui = (struct lights_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    devs_ctl = GetDlgItem(hwnd, IDC_DEVICE);
    lights_ctl = GetDlgItem(hwnd, IDC_LIGHT);
    dev_idx = ComboBox_GetCurSel(devs_ctl);

    if (dev_idx >= (int) ui->devices.nitems) {
        goto unlocked_fail;
    }

    ListView_DeleteAllItems(lights_ctl);

    hid = *array_item(struct hid_stub *, &ui->devices, dev_idx);

    hid_mgr_lock();

    if (!hid_stub_get_lights(hid, NULL, &ui->nlights)) {
        goto locked_fail;
    }

    ui->lights = xrealloc(ui->lights, sizeof(*ui->lights) * ui->nlights);

    if (!hid_stub_get_lights(hid, ui->lights, &ui->nlights)) {
        free(ui->lights);
        ui->lights = NULL;

        goto locked_fail;
    }

    for (i = 0; i < ui->nlights; i++) {
        if (ui->lights[i].name[0] != L'\0') {
            item.pszText = ui->lights[i].name;
        } else {
            wchars[0] = L'\0';

            usages_get(chars, lengthof(chars), ui->lights[i].usage);
            MultiByteToWideChar(
                CP_UTF8, 0, chars, lengthof(chars), wchars, lengthof(wchars));

            item.pszText = wchars;
        }

        item.mask = LVIF_TEXT;
        item.iItem = (int) i;
        item.iSubItem = 0;

        ListView_InsertItem(lights_ctl, &item);
    }

    ui->hid = hid;
    lights_update_bindings(hwnd);

locked_fail:
    hid_mgr_unlock();

unlocked_fail:
    return TRUE;
}

static INT_PTR lights_handle_highlight_light(HWND hwnd, const NMLISTVIEW *n)
{
    size_t light_no;

    light_no = n->iItem;

    if (n->uNewState & LVIS_SELECTED) {
        lights_pulse_start(hwnd, light_no);
    }

    return TRUE;
}

static INT_PTR lights_handle_bind_light(HWND hwnd)
{
    struct lights_ui *ui;
    struct mapped_light ml;
    uint8_t game_light;
    bool bound;
    HINSTANCE inst;
    HWND lights_ctl;
    int pos;

    ui = (struct lights_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    lights_ctl = GetDlgItem(hwnd, IDC_LIGHT);

    pos = ListView_GetSelectionMark(lights_ctl);

    if (pos < 0) {
        return TRUE;
    }

    ml.hid = ui->hid;
    ml.light_no = pos;

    if (bind_light(inst, hwnd, ui->schema, &ml, &game_light, &bound)) {
        if (bound) {
            mapper_set_light_map(&ml, game_light);
        } else {
            mapper_clear_light_map(&ml);
        }

        lights_update_bindings(hwnd);
    }

    return TRUE;
}

static INT_PTR lights_handle_pulse_tick(HWND hwnd)
{
    struct lights_ui *ui;
    size_t light_no;
    float intensity;
    float bias;
    float scale;
    float tmp;

    ui = (struct lights_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (!ui->pulse_active) {
        return TRUE;
    }

    light_no = ui->pulse_light_no;

    if (ui->hid == NULL || light_no >= ui->nlights) {
        return TRUE;
    }

    if (ui->pulse_up) {
        tmp = ui->pulse_coeff + PULSE_DELTA;

        if (tmp > 1.0f) {
            ui->pulse_coeff = 1.0f;
            ui->pulse_up = false;
        } else {
            ui->pulse_coeff = tmp;
        }
    } else {
        tmp = ui->pulse_coeff - PULSE_DELTA;

        if (tmp < 0.0f) {
            ui->pulse_coeff = 0.0f;
            ui->pulse_up = true;
        } else {
            ui->pulse_coeff = tmp;
        }
    }

    bias = (float) (ui->lights[light_no].value_min);
    scale = (float) (ui->lights[light_no].value_max -
                     ui->lights[light_no].value_min);

    /* Intensity perception is non-linear. Pulse quadratically. */

    intensity = bias + scale * ui->pulse_coeff * ui->pulse_coeff + 0.5f;

    hid_mgr_lock();
    hid_stub_set_light(ui->hid, light_no, (int32_t) intensity);
    hid_mgr_unlock();

    return TRUE;
}

static void lights_update_bindings(HWND hwnd)
{
    light_iter_t pos;
    struct mapped_light ml;
    uint8_t game_light;
    size_t i;
    wchar_t wchars[256];
    struct lights_ui *ui;
    HINSTANCE inst;
    HWND lights_ctl;

    ui = (struct lights_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    lights_ctl = GetDlgItem(hwnd, IDC_LIGHT);

    for (i = 0; i < ui->nlights; i++) {
        ListView_SetItemText(lights_ctl, i, 1, NULL);
    }

    for (pos = mapper_iterate_lights(); light_iter_is_valid(pos);
         light_iter_next(pos)) {
        light_iter_get_mapping(pos, &ml);

        if (ml.hid != ui->hid) {
            continue;
        }

        if (ml.light_no >= ui->nlights) {
            /*  Shouldn't happen unless the HID report descriptor has changed
                since we last configured this device */

            continue;
        }

        game_light = light_iter_get_game_light(pos);

        for (i = 0; i < ui->schema->nlights; i++) {
            if (ui->schema->lights[i].bit == game_light) {
                LoadString(
                    inst,
                    ui->schema->lights[i].name_rsrc,
                    wchars,
                    lengthof(wchars));
                ListView_SetItemText(lights_ctl, ml.light_no, 1, wchars);
            }
        }
    }

    light_iter_free(pos);
}

static void lights_pulse_start(HWND hwnd, size_t light_no)
{
    struct lights_ui *ui;
    int32_t intensity;

    ui = (struct lights_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (ui->pulse_active) {
        lights_pulse_stop(hwnd);
    }

    /* Start light at max intensity */

    intensity = ui->lights[light_no].value_max;

    hid_mgr_lock();
    hid_stub_set_light(ui->hid, light_no, intensity);
    hid_mgr_unlock();

    ui->pulse_active = true;
    ui->pulse_up = false;
    ui->pulse_light_no = light_no;
    ui->pulse_coeff = 1.0f;

    SetTimer(hwnd, 1, 17, NULL);
}

static void lights_pulse_stop(HWND hwnd)
{
    struct lights_ui *ui;
    size_t light_no;
    int32_t intensity;

    ui = (struct lights_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (!ui->pulse_active) {
        return;
    }

    light_no = ui->pulse_light_no;

    if (ui->hid != NULL && light_no < ui->nlights) {
        intensity = ui->lights[light_no].value_min;

        hid_mgr_lock();
        hid_stub_set_light(ui->hid, light_no, intensity);
        hid_mgr_unlock();
    }

    ui->pulse_active = false;

    KillTimer(hwnd, 1);
}

static INT_PTR lights_handle_fini(HWND hwnd)
{
    struct lights_ui *ui;

    lights_pulse_stop(hwnd);

    ui = (struct lights_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    array_fini(&ui->devices);
    free(ui->lights);
    free(ui);

    return TRUE;
}
