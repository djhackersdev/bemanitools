#define LOG_MODULE "mouse"

#include <initguid.h>
#include <windows.h>

#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "geninput/hid.h"
#include "geninput/mouse.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"

enum mouse_ctl_no {
    MOUSE_CTL_BTN1,
    MOUSE_CTL_BTN2,
    MOUSE_CTL_BTN3,
    MOUSE_CTL_BTN4,
    MOUSE_CTL_BTN5,
    MOUSE_CTL_X,
    MOUSE_CTL_Y,
    MOUSE_CTL_WHEEL,

    MOUSE_CTL_COUNT
};

struct mouse {
    struct hid_ri super;
    wchar_t *name;
    long last_x;
    long last_y;
    long values[MOUSE_CTL_COUNT];
};

static const struct hid_control mouse_controls[MOUSE_CTL_COUNT] = {
    /* Left, right, middle, AUX1, AUX2 buttons */
    {0x00090001, 0x00000000, 0x00000001, 0},
    {0x00090002, 0x00000000, 0x00000001, 0},
    {0x00090003, 0x00000000, 0x00000001, 0},
    {0x00090004, 0x00000000, 0x00000001, 0},
    {0x00090005, 0x00000000, 0x00000001, 0},

    /* X axis */
    {0x00010030, 0xC0000000, 0x3FFFFFFF, HID_FLAG_RELATIVE},

    /* Y axis */
    {0x00010031, 0xC0000000, 0x3FFFFFFF, HID_FLAG_RELATIVE},

    /* Mouse wheel */
    {0x00010038, 0xFFFF8000, 0x00007FFF, HID_FLAG_RELATIVE},
};

static const uint16_t mouse_down_flg[] = {
    RI_MOUSE_BUTTON_1_DOWN,
    RI_MOUSE_BUTTON_2_DOWN,
    RI_MOUSE_BUTTON_3_DOWN,
    RI_MOUSE_BUTTON_4_DOWN,
    RI_MOUSE_BUTTON_5_DOWN,
};

static const uint16_t mouse_up_flg[] = {
    RI_MOUSE_BUTTON_1_UP,
    RI_MOUSE_BUTTON_2_UP,
    RI_MOUSE_BUTTON_3_UP,
    RI_MOUSE_BUTTON_4_UP,
    RI_MOUSE_BUTTON_5_UP,
};

DEFINE_GUID(
    mouse_guid,
    0x378DE44C,
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

static bool mouse_get_device_usage(const struct hid *hid, uint32_t *usage);
static bool
mouse_get_name(const struct hid *hid, wchar_t *chars, size_t *nchars);
static bool mouse_get_controls(
    const struct hid *hid, struct hid_control *controls, size_t *ncontrols);
static bool mouse_get_lights(
    const struct hid *hid, struct hid_light *lights, size_t *nlights);
static bool
mouse_get_value(struct hid *hid, size_t control_no, int32_t *out_value);
static bool
mouse_set_light(struct hid *hid, size_t light_no, uint32_t intensity);
static void mouse_handle_event(struct hid_ri *hid_ri, const RAWINPUT *ri);
static void mouse_close(struct hid *hid);

static const struct hid_ri_vtbl mouse_vtbl = {
    {/* .close               = */ mouse_close,
     /* .get_device_usage    = */ mouse_get_device_usage,
     /* .get_name            = */ mouse_get_name,
     /* .get_controls        = */ mouse_get_controls,
     /* .get_lights          = */ mouse_get_lights,
     /* .get_value           = */ mouse_get_value,
     /* .set_light           = */ mouse_set_light},
    /* .handle_event        = */ mouse_handle_event};

void mouse_create(struct hid_ri **hid_ri, const char *dev_node)
{
    struct mouse *m;
    char *tmp;

    m = xcalloc(sizeof(*m));
    m->super.vptr = &mouse_vtbl;
    m->name = hid_ri_init_name(&mouse_guid, dev_node);

    *hid_ri = &m->super;

    if (!wstr_narrow(m->name, &tmp)) {
        tmp = str_dup("???");
    }

    log_misc("Opened mouse on dev node %s", dev_node);
    log_misc("... Product: %s", tmp);

    free(tmp);
}

static bool mouse_get_device_usage(const struct hid *hid, uint32_t *usage)
{
    *usage = MOUSE_DEVICE_USAGE;

    return true;
}

static bool
mouse_get_name(const struct hid *hid, wchar_t *chars, size_t *nchars)
{
    struct mouse *m = containerof(hid, struct mouse, super);

    return hid_ri_get_name(chars, nchars, m->name);
}

static bool mouse_get_controls(
    const struct hid *hid, struct hid_control *controls, size_t *ncontrols)
{
    return hid_ri_get_controls(
        controls, ncontrols, mouse_controls, MOUSE_CTL_COUNT);
}

static bool mouse_get_lights(
    const struct hid *hid, struct hid_light *lights, size_t *nlights)
{
    log_assert(nlights != NULL);

    if (lights == NULL) {
        *nlights = 0;
    }

    return true;
}

static bool
mouse_get_value(struct hid *hid, size_t control_no, int32_t *out_value)
{
    struct mouse *m = containerof(hid, struct mouse, super);
    long tmp;

    log_assert(control_no < MOUSE_CTL_COUNT);

    switch (control_no) {
        case MOUSE_CTL_BTN1:
        case MOUSE_CTL_BTN2:
        case MOUSE_CTL_BTN3:
        case MOUSE_CTL_BTN4:
        case MOUSE_CTL_BTN5:
            *out_value = m->values[control_no];

            return true;

        case MOUSE_CTL_X:
        case MOUSE_CTL_Y:
        case MOUSE_CTL_WHEEL:
            do {
                tmp = m->values[control_no];
            } while (InterlockedCompareExchange(
                         &m->values[control_no], 0, tmp) != tmp);

            *out_value = tmp;

            return true;

        default:
            return false;
    }
}

static bool
mouse_set_light(struct hid *hid, size_t light_no, uint32_t intensity)
{
    return false;
}

static void mouse_handle_event(struct hid_ri *hid_ri, const RAWINPUT *ri)
{
    struct mouse *m = containerof(hid_ri, struct mouse, super);
    long tmp;
    long dwheel;
    long dx;
    long dy;
    int i;

    if (ri->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
        dx = ri->data.mouse.lLastX - m->last_x;
        dy = ri->data.mouse.lLastY - m->last_y;

        m->last_x = ri->data.mouse.lLastX;
        m->last_y = ri->data.mouse.lLastY;
    } else {
        dx = ri->data.mouse.lLastX;
        dy = ri->data.mouse.lLastY;

        m->last_x += dx;
        m->last_y += dy;
    }

    do {
        tmp = m->values[MOUSE_CTL_X];
    } while (InterlockedCompareExchange(
                 &m->values[MOUSE_CTL_X], tmp + dx, tmp) != tmp);

    do {
        tmp = m->values[MOUSE_CTL_Y];
    } while (InterlockedCompareExchange(
                 &m->values[MOUSE_CTL_Y], tmp + dy, tmp) != tmp);

    if (ri->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
        dwheel = ri->data.mouse.usButtonData;

        do {
            tmp = m->values[MOUSE_CTL_WHEEL];
        } while (InterlockedCompareExchange(
                     &m->values[MOUSE_CTL_WHEEL], tmp + dwheel, tmp) != tmp);
    }

    for (i = 0; i < 5; i++) {
        if (ri->data.mouse.usButtonFlags & mouse_down_flg[i]) {
            m->values[i] = 1;
        } else if (ri->data.mouse.usButtonFlags & mouse_up_flg[i]) {
            m->values[i] = 0;
        }
    }
}

static void mouse_close(struct hid *hid)
{
    struct mouse *m = containerof(hid, struct mouse, super);

    free(m->name);
    free(m);
}
