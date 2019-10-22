// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <windowsx.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "config/resource.h"
#include "config/schema.h"
#include "config/usages.h"

#include "geninput/hid-mgr.h"
#include "geninput/mapper.h"

#include "util/defs.h"
#include "util/mem.h"

struct bind_light_state {
    const struct schema *schema;
    const struct mapped_light *ml;
    uint8_t *game_light;
    bool *bound;
};

static INT_PTR CALLBACK
bind_light_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static INT_PTR
bind_light_handle_init(HWND hwnd, struct bind_light_state *state);
static INT_PTR bind_light_handle_ok(HWND hwnd);

bool bind_light(
    HINSTANCE inst,
    HWND hwnd,
    const struct schema *schema,
    const struct mapped_light *ml,
    uint8_t *game_light,
    bool *bound)
{
    struct bind_light_state state;

    state.schema = schema;
    state.ml = ml;
    state.game_light = game_light;
    state.bound = bound;

    return DialogBoxParam(
               inst,
               MAKEINTRESOURCE(IDD_BIND_LIGHT),
               hwnd,
               bind_light_dlg_proc,
               (LPARAM) &state) != 0;
}

static INT_PTR CALLBACK
bind_light_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_INITDIALOG:
            return bind_light_handle_init(
                hwnd, (struct bind_light_state *) lparam);

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDOK:
                    switch (HIWORD(wparam)) {
                        case BN_CLICKED:
                            return bind_light_handle_ok(hwnd);
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
    }

    return FALSE;
}

static INT_PTR bind_light_handle_init(HWND hwnd, struct bind_light_state *state)
{
    char chars[256];
    wchar_t wchars[256];
    struct hid_light *lights;
    size_t nlights;
    size_t i;
    HINSTANCE inst;
    HWND hid_light_ctl;
    HWND game_light_ctl;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (INT_PTR) state);

    inst = (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    game_light_ctl = GetDlgItem(hwnd, IDC_GAME_LIGHT);

    ComboBox_AddString(game_light_ctl, L"");

    for (i = 0; i < state->schema->nlights; i++) {
        LoadString(
            inst, state->schema->lights[i].name_rsrc, wchars, lengthof(wchars));
        ComboBox_AddString(game_light_ctl, wchars);

        if (state->bound &&
            state->schema->lights[i].bit == *state->game_light) {
            ComboBox_SetCurSel(game_light_ctl, i + 1);
        }
    }

    hid_light_ctl = GetDlgItem(hwnd, IDC_LIGHT);

    hid_mgr_lock();

    if (!hid_stub_get_lights(state->ml->hid, NULL, &nlights)) {
        goto size_fail;
    }

    if (state->ml->light_no >= nlights) {
        goto bounds_fail;
    }

    lights = xmalloc(sizeof(*lights) * nlights);

    if (!hid_stub_get_lights(state->ml->hid, lights, &nlights)) {
        goto data_fail;
    }

    usages_get(chars, lengthof(chars), lights[state->ml->light_no].usage);
    MultiByteToWideChar(CP_UTF8, 0, chars, -1, wchars, lengthof(wchars));

    SetWindowText(hid_light_ctl, wchars);

data_fail:
    free(lights);

bounds_fail:
size_fail:
    hid_mgr_unlock();

    return TRUE;
}

static INT_PTR bind_light_handle_ok(HWND hwnd)
{
    struct bind_light_state *state;
    int pos;
    HWND game_light_ctl;

    state = (struct bind_light_state *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    game_light_ctl = GetDlgItem(hwnd, IDC_GAME_LIGHT);
    pos = ComboBox_GetCurSel(game_light_ctl);

    if (pos <= 0) {
        *state->bound = false;
    } else {
        *state->bound = true;
        *state->game_light = state->schema->lights[pos - 1].bit;
    }

    EndDialog(hwnd, TRUE);

    return TRUE;
}
