#include <string.h>
#include <wchar.h>

#include "geninput/dev-list.h"
#include "geninput/hid.h"

#include "util/log.h"
#include "util/str.h"

wchar_t *hid_ri_init_name(const GUID *class_guid, const char *dev_node)
{
    struct dev_list devs;
    wchar_t *name;

    dev_list_init(&devs, class_guid);

    name = NULL;

    while (dev_list_next(&devs)) {
        if (_stricmp(dev_list_get_dev_node(&devs), dev_node) == 0) {
            name = wstr_dup(dev_list_get_dev_name(&devs));

            break;
        }
    }

    if (name == NULL) {
        /* Shouldn't ever happen, but... */
        name = wstr_dup(L"???");
    }

    dev_list_fini(&devs);

    return name;
}

bool hid_ri_get_name(wchar_t *chars, size_t *nchars, const wchar_t *src_chars)
{
    size_t len;

    log_assert(nchars != NULL);

    len = src_chars != NULL ? wcslen(src_chars) + 1 : 0;

    if (chars == NULL) {
        *nchars = len;

        return true;
    } else if (*nchars >= len) {
        memcpy(chars, src_chars, len * sizeof(wchar_t));

        return true;
    } else {
        return false;
    }
}

bool hid_ri_get_controls(
    struct hid_control *controls,
    size_t *ncontrols,
    const struct hid_control *src_controls,
    size_t src_ncontrols)
{
    log_assert(ncontrols != NULL);

    if (controls == NULL) {
        *ncontrols = src_ncontrols;

        return true;
    } else if (*ncontrols >= src_ncontrols) {
        *ncontrols = src_ncontrols;

        memcpy(controls, src_controls, src_ncontrols * sizeof(*src_controls));

        return true;
    } else {
        return false;
    }
}
