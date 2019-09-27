#include <stdlib.h>

#include <windows.h>

#include "geninput/hid-mgr.h"

#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"

struct hid_stub {
    struct hid_stub *next;
    char *dev_node;
    struct hid *hid;
};

static CRITICAL_SECTION hid_mgr_cs;
static struct hid_stub *hid_stubs;

static bool hid_stub_check(struct hid_stub *stub, bool ok);

void hid_mgr_init(void)
{
    InitializeCriticalSection(&hid_mgr_cs);
    hid_stubs = NULL;
}

struct hid_stub *hid_mgr_get_first_stub(void)
{
    return hid_stubs;
}

struct hid_stub *hid_mgr_get_next_stub(struct hid_stub *stub)
{
    return stub->next;
}

struct hid_stub *hid_mgr_get_named_stub(const char *dev_node)
{
    struct hid_stub *pos;

    for (pos = hid_stubs ; pos != NULL ; pos = pos->next) {
        if (_stricmp(pos->dev_node, dev_node) == 0) {
            return pos;
        }
    }

    pos = xmalloc(sizeof(*pos));

    pos->next = hid_stubs;
    pos->dev_node = str_dup(dev_node);
    pos->hid = NULL;

    hid_stubs = pos;

    return pos;
}

void hid_mgr_lock(void)
{
    EnterCriticalSection(&hid_mgr_cs);
}

void hid_mgr_unlock(void)
{
    LeaveCriticalSection(&hid_mgr_cs);
}

void hid_mgr_fini(void)
{
    struct hid_stub *next;
    struct hid_stub *pos;

    for (pos = hid_stubs ; pos != NULL ; pos = next) {
        next = pos->next;

        if (pos->hid != NULL) {
            hid_close(pos->hid);
        }

        free(pos->dev_node);
        free(pos);
    }

    DeleteCriticalSection(&hid_mgr_cs);
}

static bool hid_stub_check(struct hid_stub *stub, bool ok)
{
    if (!ok) {
        log_warning("Detaching erroring HID at %s", stub->dev_node);
        hid_stub_detach(stub);
    }

    return ok;
}

void hid_stub_attach(struct hid_stub *stub, struct hid *hid)
{
    if (stub->hid != NULL) {
        hid_close(stub->hid);
    }

    stub->hid = hid;
}

void hid_stub_detach(struct hid_stub *stub)
{
    if (stub->hid != NULL) {
        hid_close(stub->hid);
    }

    stub->hid = NULL;
}

bool hid_stub_is_attached(struct hid_stub *stub)
{
    return stub->hid != NULL;
}

bool hid_stub_get_device_usage(struct hid_stub *stub, uint32_t *usage)
{
    if (stub->hid == NULL) {
        return false;
    }

    return hid_stub_check(stub,
            hid_get_device_usage(stub->hid, usage));
}

bool hid_stub_get_controls(struct hid_stub *stub, struct hid_control *controls,
        size_t *ncontrols)
{
    if (stub->hid == NULL) {
        return false;
    }

    return hid_stub_check(stub,
            hid_get_controls(stub->hid, controls, ncontrols));
}

bool hid_stub_get_lights(struct hid_stub *stub, struct hid_light *lights,
        size_t *nlights)
{
    if (stub->hid == NULL) {
        return false;
    }

    return hid_stub_check(stub,
            hid_get_lights(stub->hid, lights, nlights));
}

const char *hid_stub_get_dev_node(struct hid_stub *stub)
{
    return stub->dev_node;
}

bool hid_stub_get_name(struct hid_stub *stub, wchar_t *chars, size_t *nchars)
{
    if (stub->hid == NULL) {
        return false;
    }

    return hid_stub_check(stub,
            hid_get_name(stub->hid, chars, nchars));
}

bool hid_stub_get_value(struct hid_stub *stub, size_t control_no,
        int32_t *out_value)
{
    if (stub->hid == NULL) {
        return false;
    }

    return hid_stub_check(stub,
            hid_get_value(stub->hid, control_no, out_value));
}

bool hid_stub_set_light(struct hid_stub *stub, size_t light_no,
        uint32_t intensity)
{
    if (stub->hid == NULL) {
        return false;
    }

    return hid_stub_check(stub,
            hid_set_light(stub->hid, light_no, intensity));
}

bool hid_stub_handle_completion(struct hid_stub *stub, OVERLAPPED *ovl,
        size_t nbytes)
{
    if (stub->hid == NULL) {
        return false;
    }

    return hid_stub_check(stub,
            hid_fd_handle_completion((struct hid_fd *) stub->hid, ovl, nbytes));
}

