#ifndef LAUNCHER_PROPERTY_H
#define LAUNCHER_PROPERTY_H

#include "imports/avs.h"

struct property *boot_property_load(const char *filename);
void boot_property_free(struct property *prop);

#endif
