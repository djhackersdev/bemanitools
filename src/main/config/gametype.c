#include <windows.h>

#include <string.h>

#include "config/gametype.h"
#include "config/resource.h"
#include "config/schema.h"

#include "util/defs.h"
#include "util/log.h"

static INT_PTR CALLBACK
game_type_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static INT_PTR game_type_handle_init(HWND hwnd);
static INT_PTR game_type_handle_ok(HWND hwnd);
static INT_PTR game_type_handle_cancel(HWND hwnd);

const struct schema *game_type_from_str(const char *name)
{
    size_t i;

    for (i = 0; i < nschemas; i++) {
        if (_stricmp(schemas[i].name, name) == 0) {
            return &schemas[i];
        }
    }

    return NULL;
}

const struct schema *game_type_from_dialog(HINSTANCE inst)
{
    return (struct schema *) DialogBoxParam(
        inst, MAKEINTRESOURCE(IDD_GAME_TYPE), NULL, game_type_dlg_proc, 0);
}

static INT_PTR CALLBACK
game_type_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_INITDIALOG:
            return game_type_handle_init(hwnd);

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDOK:
                    return game_type_handle_ok(hwnd);

                case IDCANCEL:
                    return game_type_handle_cancel(hwnd);

                default:
                    return FALSE;
            }

        default:
            return FALSE;
    }
}

static INT_PTR game_type_handle_init(HWND hwnd)
{
    HINSTANCE inst;
    HWND ctl;
    size_t i;
    wchar_t text[1024];

    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    ctl = GetDlgItem(hwnd, IDC_GAME_TYPE);

    for (i = 0; i < nschemas; i++) {
        LoadString(inst, schemas[i].label, text, lengthof(text));
        SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM) text);
    }

    SendMessage(ctl, CB_SETCURSEL, 0, 0);

    return TRUE;
}

static INT_PTR game_type_handle_ok(HWND hwnd)
{
    HWND ctl;
    size_t i;

    ctl = GetDlgItem(hwnd, IDC_GAME_TYPE);
    i = (size_t) SendMessage(ctl, CB_GETCURSEL, 0, 0);

    log_assert(i < nschemas);

    EndDialog(hwnd, (INT_PTR) &schemas[i]);

    return TRUE;
}

static INT_PTR game_type_handle_cancel(HWND hwnd)
{
    EndDialog(hwnd, (INT_PTR) NULL);

    return TRUE;
}
