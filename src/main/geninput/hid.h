#ifndef GENINPUT_HID_H
#define GENINPUT_HID_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#include <windows.h>

#define HID_FLAG_NULLABLE 0x01
#define HID_FLAG_RELATIVE 0x02

DEFINE_GUID(
    hid_guid,
    0x4D1E55B2L,
    0xF16F,
    0x11CF,
    0x88,
    0xCB,
    0x00,
    0x11,
    0x11,
    0x00,
    0x00,
    0x30);

/* Alright, I'd take the vtable stuff from C++, because this is just gross.
   But you can keep everything else from that trainwreck of a language. */

struct hid;

struct hid_control {
    uint32_t usage;
    int32_t value_min;
    int32_t value_max;
    uint32_t flags;
};

struct hid_light {
    uint32_t usage;

    /*  I'm going to go ahead and assume nobody will declare a negative light
        intensity on their lighting device.

        I will probably regret this decision. */

    uint32_t value_min;
    uint32_t value_max;

    wchar_t name[126];
};

struct hid_vtbl {
    void (*close)(struct hid *hid);
    bool (*get_device_usage)(const struct hid *hid, uint32_t *usage);
    bool (*get_name)(const struct hid *hid, wchar_t *chars, size_t *nchars);
    bool (*get_controls)(
        const struct hid *hid, struct hid_control *controls, size_t *ncontrols);
    bool (*get_lights)(
        const struct hid *hid, struct hid_light *lights, size_t *nlights);
    bool (*get_value)(struct hid *hid, size_t control_no, int32_t *out_value);
    bool (*set_light)(struct hid *hid, size_t light_no, uint32_t intensity);
};

struct hid {
    const struct hid_vtbl *vptr;
};

#define hid_close(hid) (hid)->vptr->close(hid)

#define hid_get_device_usage(hid, usage) \
    (hid)->vptr->get_device_usage(hid, usage)

#define hid_get_name(hid, chars, nchars) \
    (hid)->vptr->get_name(hid, chars, nchars)

#define hid_get_controls(hid, controls, ncontrols) \
    (hid)->vptr->get_controls(hid, controls, ncontrols)

#define hid_get_lights(hid, lights, nlights) \
    (hid)->vptr->get_lights(hid, lights, nlights)

#define hid_get_value(hid, control_no, out_value) \
    (hid)->vptr->get_value(hid, control_no, out_value)

#define hid_set_light(hid, light_no, illuminated) \
    (hid)->vptr->set_light(hid, light_no, illuminated)

struct hid_ri;

struct hid_ri_vtbl {
    struct hid_vtbl super;
    void (*handle_event)(struct hid_ri *hid, const RAWINPUT *ri);
};

struct hid_ri {
    const struct hid_ri_vtbl *vptr;
};

wchar_t *hid_ri_init_name(const GUID *class_guid, const char *dev_node);
bool hid_ri_get_name(wchar_t *chars, size_t *nchars, const wchar_t *src_chars);
bool hid_ri_get_controls(
    struct hid_control *controls,
    size_t *ncontrols,
    const struct hid_control *src_controls,
    size_t src_ncontrols);

#define hid_ri_handle_event(hid_ri, ri) (hid_ri)->vptr->handle_event(hid_ri, ri)

struct hid_fd;

struct hid_fd_vtbl {
    struct hid_vtbl super;
    bool (*handle_completion)(
        struct hid_fd *hid, OVERLAPPED *ovl, size_t nbytes);
};

struct hid_fd {
    const struct hid_fd_vtbl *vptr;
};

#define hid_fd_handle_completion(hid_fd, ovl, nbytes) \
    (hid_fd)->vptr->handle_completion(hid_fd, ovl, nbytes)

#endif
