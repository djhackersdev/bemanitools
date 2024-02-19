#define LOG_MODULE "kbd"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <hidsdi.h>
#include <initguid.h>
// clang-format on

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/log.h"

#include "geninput/hid.h"
#include "geninput/kbd-data.h"
#include "geninput/kbd.h"

#include "util/defs.h"
#include "util/mem.h"
#include "util/str.h"

DEFINE_GUID(
    kbd_guid,
    0x884B96C3,
    0x56EF,
    0x11D1,
    0xBC,
    0x8C,
    0x00,
    0xA0,
    0xC9,
    0x14,
    0x05,
    0xDD);

static struct hid_control kbd_controls[512];
static size_t kbd_ncontrols;

static void kbd_static_init(void);
static bool kbd_get_device_usage(const struct hid *hid, uint32_t *usage);
static bool kbd_get_name(const struct hid *hid, wchar_t *chars, size_t *nchars);
static bool kbd_get_controls(
    const struct hid *hid, struct hid_control *controls, size_t *ncontrols);
static bool kbd_get_lights(
    const struct hid *hid, struct hid_light *lights, size_t *nlights);
static bool
kbd_get_value(struct hid *hid, size_t control_no, int32_t *out_value);
static bool kbd_set_light(struct hid *hid, size_t light_no, uint32_t intensity);
static void kbd_handle_event(struct hid_ri *hid_ri, const RAWINPUT *ri);
static void kbd_free(struct hid *hid);

static const struct hid_ri_vtbl kbd_vtbl = {
    {/* .free                = */ kbd_free,
     /* .get_device_usage    = */ kbd_get_device_usage,
     /* .get_name            = */ kbd_get_name,
     /* .get_controls        = */ kbd_get_controls,
     /* .get_lights          = */ kbd_get_lights,
     /* .get_value           = */ kbd_get_value,
     /* .set_light           = */ kbd_set_light},
    /* .handle_event        = */ kbd_handle_event};

struct kbd {
    struct hid_ri super;
    wchar_t *name;
    uint32_t state[512];
};

static void kbd_static_init(void)
{
    uint8_t code;
    size_t i;

    if (kbd_ncontrols == 0) {
        kbd_ncontrols = kbd_basic_nusages + kbd_ext_nusages;

        for (i = 0; i < kbd_ncontrols; i++) {
            if (i < kbd_basic_nusages) {
                code = kbd_basic_usages[i];
            } else {
                code = kbd_ext_usages[i - kbd_basic_nusages];
            }

            kbd_controls[i].usage = 0x00070000 | code;
            kbd_controls[i].value_min = 0;
            kbd_controls[i].value_max = 1;
            kbd_controls[i].flags = 0;
        }
    }
}

void kbd_create(struct hid_ri **hid_ri, const char *dev_node)
{
    struct kbd *kbd;
    char *tmp;

    kbd_static_init();

    kbd = xcalloc(sizeof(*kbd));
    kbd->super.vptr = &kbd_vtbl;
    kbd->name = hid_ri_init_name(&kbd_guid, dev_node);

    *hid_ri = &kbd->super;

    /* Output some debugging info */
    if (!wstr_narrow(kbd->name, &tmp)) {
        tmp = str_dup("???");
    }

    /* Dump diagnostics */
    log_misc("Opened keyboard on dev node %s", dev_node);
    log_misc("... Product: %s", tmp);

    free(tmp);
}

static bool kbd_get_device_usage(const struct hid *hid, uint32_t *usage)
{
    *usage = KBD_DEVICE_USAGE_KEYBOARD;

    return true;
}

static bool kbd_get_name(const struct hid *hid, wchar_t *chars, size_t *nchars)
{
    struct kbd *kbd = containerof(hid, struct kbd, super);

    return hid_ri_get_name(chars, nchars, kbd->name);
}

static bool kbd_get_controls(
    const struct hid *hid, struct hid_control *controls, size_t *ncontrols)
{
    return hid_ri_get_controls(
        controls, ncontrols, kbd_controls, kbd_ncontrols);
}

static bool
kbd_get_lights(const struct hid *hid, struct hid_light *lights, size_t *nlights)
{
    log_assert(nlights != NULL);

    *nlights = 0;

    return true;
}

static bool
kbd_get_value(struct hid *hid, size_t control_no, int32_t *out_value)
{
    struct kbd *kbd = containerof(hid, struct kbd, super);

    log_assert(control_no < kbd_ncontrols);

    *out_value = kbd->state[control_no];

    return true;
}

static bool kbd_set_light(struct hid *hid, size_t light_no, uint32_t intensity)
{
    return false;
}

static void kbd_handle_event(struct hid_ri *hid_ri, const RAWINPUT *ri)
{
    struct kbd *kbd = containerof(hid_ri, struct kbd, super);
    uint32_t value;

    value = (ri->data.keyboard.Flags & RI_KEY_BREAK) == 0;

    if (ri->data.keyboard.Flags & RI_KEY_E0) {
        if (ri->data.keyboard.MakeCode < kbd_ext_nusages) {
            kbd->state[ri->data.keyboard.MakeCode + kbd_basic_nusages] = value;
        }
    } else if (!(ri->data.keyboard.Flags & RI_KEY_E1)) {
        if (ri->data.keyboard.MakeCode < kbd_basic_nusages) {
            kbd->state[ri->data.keyboard.MakeCode] = value;
        }
    }
}

static void kbd_free(struct hid *hid)
{
    struct kbd *kbd = containerof(hid, struct kbd, super);

    free(kbd->name);
    free(kbd);
}
