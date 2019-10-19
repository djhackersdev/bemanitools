#ifndef GENINPUT_MOUSE_H
#define GENINPUT_MOUSE_H

#include "geninput/hid.h"

#define MOUSE_DEVICE_USAGE 0x00010002

void mouse_create(struct hid_ri **hid_ri, const char *dev_node);

#endif
