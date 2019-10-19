#ifndef GENINPUT_HID_GENERIC_STRINGS_H
#define GENINPUT_HID_GENERIC_STRINGS_H

struct hid_generic_strings {
    wchar_t *wstr_prod;
    wchar_t *wstr_manf;
    char *str_prod;
    char *str_manf;
};

void hid_generic_strings_init(struct hid_generic_strings *strings, HANDLE fd);
const char *hid_generic_strings_get_manf(struct hid_generic_strings *strings);
const char *hid_generic_strings_get_prod(struct hid_generic_strings *strings);
void hid_generic_strings_fini(struct hid_generic_strings *strings);

#endif
