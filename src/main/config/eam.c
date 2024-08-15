// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
// clang-format on

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "config/resource.h"
#include "config/schema.h"

#include "iface-core/log.h"
#include "iface-io/eam.h"
#include "iface/input.h"

#include "eamio/eam-config.h"

#include "geninput/hid-mgr.h"
#include "geninput/input-config.h"
#include "geninput/kbd.h"

#include "util/array.h"
#include "util/defs.h"
#include "util/mem.h"
#include "util/str.h"

/* You may or may not have noticed that there is a blank label in the Network
   tab's dialog resource definition. This is a consequence of the way that
   Windows UI styles work. WIN32K's (i.e. raw USER32's) GUI routines draw
   widgets using the Windows 95 chunky bezels and flat surfaces style, but if
   your app pulls in COMMCTRL v6 or later using an SxS manifest, then it
   overrides the built-in Win32 widgets with its own glossy versions, in
   addition to the extended control library that its API provides.

   I'm guessing that top-level windows and dialogs (the tab is a dialog) get
   subclassed as soon as one of these glossified built-ins gets WM_CREATEd
   within a window. Hence the presence of the Static control to make sure this
   takes place.

   Otherwise, you get a flat grey shaded tab body, which looks wrong on Vista
   and above. */

struct eam_ui {
    struct array children;
};

struct eam_unit_ui {
    const struct eam_unit_def *def;
    struct array devs;
};

static INT_PTR CALLBACK
eam_ui_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static INT_PTR eam_ui_handle_init(HWND hwnd, const PROPSHEETPAGE *psp);
static INT_PTR eam_ui_handle_activate(HWND hwnd);
static INT_PTR eam_ui_handle_passivate(HWND hwnd);
static INT_PTR eam_ui_handle_tick(HWND hwnd);
static INT_PTR eam_ui_handle_change_alt_10k(HWND hwnd);
static INT_PTR eam_ui_handle_change_autogen(HWND hwnd);
static INT_PTR eam_ui_handle_fini(HWND hwnd);

static INT_PTR CALLBACK
eam_unit_ui_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static INT_PTR
eam_unit_ui_handle_init(HWND hwnd, const struct eam_unit_def *def);
static void eam_unit_ui_handle_init_devs(HWND hwnd);
static void eam_unit_ui_handle_init_path(HWND hwnd);
static INT_PTR eam_unit_ui_handle_browse(HWND hwnd);
static INT_PTR eam_unit_ui_handle_change_device(HWND hwnd);
static INT_PTR eam_unit_ui_handle_tick(HWND hwnd);
static INT_PTR eam_unit_ui_handle_fini(HWND hwnd);

static const bt_io_eam_config_api_t *eam_io_config_api;

HPROPSHEETPAGE
eam_ui_tab_create(
    HINSTANCE inst,
    const struct schema *schema,
    const bt_io_eam_config_api_t *api)
{
    PROPSHEETPAGE psp;

    eam_io_config_api = api;

    memset(&psp, 0, sizeof(psp));
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = inst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_TAB_NETWORK);
    psp.pfnDlgProc = eam_ui_dlg_proc;
    psp.lParam = (LPARAM) schema;

    return CreatePropertySheetPage(&psp);
}

static INT_PTR CALLBACK
eam_ui_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    const NMHDR *n;

    switch (msg) {
        case WM_INITDIALOG:
            return eam_ui_handle_init(hwnd, (PROPSHEETPAGE *) lparam);

        case WM_TIMER:
            return eam_ui_handle_tick(hwnd);

        case WM_COMMAND:
            switch (HIWORD(wparam)) {
                case BN_CLICKED:
                    switch (LOWORD(wparam)) {
                        case IDC_AUTOGEN:
                            return eam_ui_handle_change_autogen(hwnd);

                        case IDC_ALT_10K:
                            return eam_ui_handle_change_alt_10k(hwnd);

                        default:
                            return FALSE;
                    }

                default:
                    return FALSE;
            }

        case WM_NOTIFY:
            n = (NMHDR *) lparam;

            switch (n->code) {
                case PSN_SETACTIVE:
                    return eam_ui_handle_activate(hwnd);

                case PSN_KILLACTIVE:
                    return eam_ui_handle_passivate(hwnd);
            }

            return FALSE;

        case WM_DESTROY:
            return eam_ui_handle_fini(hwnd);

        default:
            return FALSE;
    }
}

static INT_PTR eam_ui_handle_init(HWND hwnd, const PROPSHEETPAGE *psp)
{
    struct eam_ui *ui;
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

    for (i = 0; i < schema->nunits; i++) {
        child = CreateDialogParam(
            inst,
            MAKEINTRESOURCE(IDD_READER),
            hwnd,
            eam_unit_ui_dlg_proc,
            (LPARAM) &schema->units[i]);

        GetWindowRect(child, &r);
        SetWindowPos(
            child, HWND_BOTTOM, 0, ypos, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

        ypos += r.bottom - r.top;

        *array_append(HWND, &ui->children) = child;
    }

    if (eam_io_config_api->get_autogen()) {
        SendMessage(GetDlgItem(hwnd, IDC_AUTOGEN), BM_SETCHECK, BST_CHECKED, 0);
    }

    if (eam_io_config_api->get_alt_10k()) {
        SendMessage(GetDlgItem(hwnd, IDC_ALT_10K), BM_SETCHECK, BST_CHECKED, 0);
    }

    return TRUE;
}

static INT_PTR eam_ui_handle_activate(HWND hwnd)
{
    SetTimer(hwnd, 17, 1, NULL);

    return TRUE;
}

static INT_PTR eam_ui_handle_passivate(HWND hwnd)
{
    KillTimer(hwnd, 1);

    return TRUE;
}

static INT_PTR eam_ui_handle_tick(HWND hwnd)
{
    HWND child;
    struct eam_ui *ui;
    size_t i;

    ui = (struct eam_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    bt_input_mapper_update();

    for (i = 0; i < ui->children.nitems; i++) {
        child = *array_item(HWND, &ui->children, i);
        SendMessage(child, WM_USER, 0, 0);
    }

    return TRUE;
}

static INT_PTR eam_ui_handle_change_alt_10k(HWND hwnd)
{
    HWND btn;
    bool alt_10k;

    btn = GetDlgItem(hwnd, IDC_ALT_10K);
    alt_10k = SendMessage(btn, BM_GETCHECK, 0, 0) == BST_CHECKED;

    eam_io_config_api->set_alt_10k(alt_10k);

    return TRUE;
}

static INT_PTR eam_ui_handle_change_autogen(HWND hwnd)
{
    HWND btn;
    bool autogen;

    btn = GetDlgItem(hwnd, IDC_AUTOGEN);
    autogen = SendMessage(btn, BM_GETCHECK, 0, 0) == BST_CHECKED;

    eam_io_config_api->set_autogen(autogen);

    return TRUE;
}

static INT_PTR eam_ui_handle_fini(HWND hwnd)
{
    struct eam_ui *ui;

    ui = (struct eam_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    array_fini(&ui->children);
    free(ui);

    return TRUE;
}

static INT_PTR CALLBACK
eam_unit_ui_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_INITDIALOG:
            return eam_unit_ui_handle_init(
                hwnd, (struct eam_unit_def *) lparam);

        case WM_USER:
            return eam_unit_ui_handle_tick(hwnd);

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDC_KBD_DEVICE:
                    switch (HIWORD(wparam)) {
                        case CBN_SELCHANGE:
                            return eam_unit_ui_handle_change_device(hwnd);

                        default:
                            return FALSE;
                    }

                case IDC_BROWSE:
                    switch (HIWORD(wparam)) {
                        case BN_CLICKED:
                            return eam_unit_ui_handle_browse(hwnd);

                        default:
                            return FALSE;
                    }

                default:
                    return FALSE;
            }

        case WM_DESTROY:
            return eam_unit_ui_handle_fini(hwnd);

        default:
            return FALSE;
    }
}

static INT_PTR
eam_unit_ui_handle_init(HWND hwnd, const struct eam_unit_def *def)
{
    struct eam_unit_ui *ui;
    wchar_t str[128];
    HINSTANCE inst;

    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

    LoadString(inst, def->label_rsrc, str, lengthof(str));
    SetWindowText(GetDlgItem(hwnd, IDC_GROUP), str);

    ui = xmalloc(sizeof(*ui));
    ui->def = def;
    array_init(&ui->devs);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) ui);

    eam_unit_ui_handle_init_devs(hwnd);
    eam_unit_ui_handle_init_path(hwnd);

    return TRUE;
}

static void eam_unit_ui_handle_init_devs(HWND hwnd)
{
    struct eam_unit_ui *ui;
    uint32_t dev_usage;
    wchar_t *dev_name;
    struct hid_stub *hid;
    size_t nchars;
    LRESULT index;
    HWND dev_list;

    ui = (struct eam_unit_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    dev_list = GetDlgItem(hwnd, IDC_KBD_DEVICE);

    *array_append(struct hid_stub *, &ui->devs) = NULL;
    SendMessage(dev_list, CB_ADDSTRING, 0, (LPARAM) TEXT(""));

    hid_mgr_lock();

    for (hid = hid_mgr_get_first_stub(); hid != NULL;
         hid = hid_mgr_get_next_stub(hid)) {
        if (!hid_stub_get_device_usage(hid, &dev_usage)) {
            continue;
        }

        if (dev_usage != KBD_DEVICE_USAGE_KEYBOARD &&
            dev_usage != KBD_DEVICE_USAGE_KEYPAD) {
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

        *array_append(struct hid_stub *, &ui->devs) = hid;

        if (hid == eam_io_config_api->get_keypad_device(ui->def->unit_no)) {
            SendMessage(dev_list, CB_SETCURSEL, index, 0);
        }
    }

    hid_mgr_unlock();
}

static void eam_unit_ui_handle_init_path(HWND hwnd)
{
    struct eam_unit_ui *ui;
    const char *path;
    wchar_t *wpath;

    ui = (struct eam_unit_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    path = eam_io_config_api->get_card_path(ui->def->unit_no);

    if (path != NULL) {
        wpath = str_widen(path);
        SetWindowText(GetDlgItem(hwnd, IDC_CARD_PATH), wpath);
        free(wpath);
    }
}

static INT_PTR eam_unit_ui_handle_browse(HWND hwnd)
{
    struct eam_unit_ui *ui;
    char *path_tmp;
    OPENFILENAME ofn;
    TCHAR path[MAX_PATH];
    HWND path_ctl;

    ui = (struct eam_unit_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    path_ctl = GetDlgItem(hwnd, IDC_CARD_PATH);

    path[0] = 0;
    GetWindowText(path_ctl, path, lengthof(path));

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = lengthof(path);
    ofn.Flags = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_EXPLORER;

    if (GetOpenFileName(&ofn) && wstr_narrow(path, &path_tmp)) {
        eam_io_config_api->set_card_path(ui->def->unit_no, path_tmp);
        free(path_tmp);

        SendMessage(path_ctl, WM_SETTEXT, 0, (LPARAM) path);
    }

    return TRUE;
}

static INT_PTR eam_unit_ui_handle_change_device(HWND hwnd)
{
    struct eam_unit_ui *ui;
    struct hid_stub *hid;
    HWND dev_list;
    LRESULT index;

    ui = (struct eam_unit_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    dev_list = GetDlgItem(hwnd, IDC_KBD_DEVICE);

    index = SendMessage(dev_list, CB_GETCURSEL, 0, 0);

    if (index != CB_ERR) {
        log_assert((size_t) index < ui->devs.nitems);

        hid = *array_item(struct hid_stub *, &ui->devs, index);
        eam_io_config_api->set_keypad_device(ui->def->unit_no, hid);
    }

    return TRUE;
}

static INT_PTR eam_unit_ui_handle_tick(HWND hwnd)
{
    struct eam_unit_ui *ui;
    uint16_t state;
    wchar_t state_str[9];

    ui = (struct eam_unit_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    state = bt_io_eam_keypad_state_get(ui->def->unit_no);

    wstr_format(state_str, lengthof(state_str), L"%04x", state);

    SetWindowText(GetDlgItem(hwnd, IDC_KEYPAD_STATE), state_str);

    return TRUE;
}

static INT_PTR eam_unit_ui_handle_fini(HWND hwnd)
{
    struct eam_unit_ui *ui;
    wchar_t path[MAX_PATH];
    char *tmp;

    ui = (struct eam_unit_ui *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    path[0] = 0;
    GetWindowText(GetDlgItem(hwnd, IDC_CARD_PATH), path, lengthof(path));

    if (path[0]) {
        if (wstr_narrow(path, &tmp)) {
            eam_io_config_api->set_card_path(ui->def->unit_no, tmp);
            free(tmp);
        }
    } else {
        eam_io_config_api->set_card_path(ui->def->unit_no, NULL);
    }

    array_fini(&ui->devs);
    free(ui);

    return TRUE;
}
