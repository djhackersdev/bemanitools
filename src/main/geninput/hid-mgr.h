#ifndef GENINPUT_HID_MGR_H
#define GENINPUT_HID_MGR_H

#include <windows.h>

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#include "geninput/hid.h"

void hid_mgr_init(void);
void hid_mgr_lock(void);
void hid_mgr_unlock(void);
struct hid_stub *hid_mgr_get_first_stub(void);
struct hid_stub *hid_mgr_get_next_stub(struct hid_stub *stub);
struct hid_stub *hid_mgr_get_named_stub(const char *dev_node);
void hid_mgr_fini(void);

const char *hid_stub_get_dev_node(struct hid_stub *stub);
void hid_stub_attach(struct hid_stub *stub, struct hid *hid);
void hid_stub_detach(struct hid_stub *stub);
bool hid_stub_get_name(struct hid_stub *stub, wchar_t *name, size_t *nchars);
bool hid_stub_is_attached(struct hid_stub *stub);
bool hid_stub_get_device_usage(struct hid_stub *stub, uint32_t *usage);
bool hid_stub_get_controls(
    struct hid_stub *stub, struct hid_control *controls, size_t *ncontrols);
bool hid_stub_get_lights(
    struct hid_stub *stub, struct hid_light *lights, size_t *nlights);
bool hid_stub_get_value(
    struct hid_stub *stub, size_t control_no, int32_t *out_value);
bool hid_stub_set_light(
    struct hid_stub *stub, size_t light_no, uint32_t intensity);
bool hid_stub_handle_completion(
    struct hid_stub *stub, OVERLAPPED *ovl, size_t nbytes);

#endif
