#ifndef GENINPUT_KBD_H
#define GENINPUT_KBD_H

#include "geninput/hid.h"

#define KBD_DEVICE_USAGE_KEYBOARD       0x00010006
#define KBD_DEVICE_USAGE_KEYPAD         0x00010007

void kbd_create(struct hid_ri **hid_ri, const char *dev_node);

#endif
