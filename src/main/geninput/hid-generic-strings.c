#include <hidsdi.h>
#include <windows.h>

#include <stdlib.h>

#include "geninput/hid-generic-strings.h"

#include "util/defs.h"
#include "util/str.h"

void hid_generic_strings_init(struct hid_generic_strings *strings, HANDLE fd)
{
    wchar_t wstr[128];

    wstr_cpy(wstr, lengthof(wstr), L"???");
    HidD_GetProductString(fd, wstr, sizeof(wstr));
    strings->wstr_prod = wstr_dup(wstr);

    if (!wstr_narrow(wstr, &strings->str_prod)) {
        strings->str_prod = str_dup("???");
    }

    wstr_cpy(wstr, lengthof(wstr), L"???");
    HidD_GetManufacturerString(fd, wstr, sizeof(wstr));
    strings->wstr_manf = wstr_dup(wstr);

    if (!wstr_narrow(wstr, &strings->str_manf)) {
        strings->str_manf = str_dup("???");
    }
}

const char *hid_generic_strings_get_manf(struct hid_generic_strings *strings)
{
    return strings->str_manf;
}

const char *hid_generic_strings_get_prod(struct hid_generic_strings *strings)
{
    return strings->str_prod;
}

void hid_generic_strings_fini(struct hid_generic_strings *strings)
{
    free(strings->str_manf);
    free(strings->wstr_manf);
    free(strings->str_prod);
    free(strings->wstr_prod);
}
