#ifndef GENINPUT_KBD_H
#define GENINPUT_KBD_H

#include "geninput/hid.h"

#define KBD_DEVICE_USAGE_KEYBOARD 0x00010006
#define KBD_DEVICE_USAGE_KEYPAD 0x00010007

/**
 * Creates a keyboard device for use with key bindings
 * 
 * @note name_override can be NULL and is only intended to be populated when
 * the desired device name is not the name as indicated by the dev_node
 */
void kbd_create(
    struct hid_ri **hid_ri, const char *dev_node, const wchar_t *name_override);

#endif
