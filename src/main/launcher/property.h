#ifndef LAUNCHER_PROPERTY_H
#define LAUNCHER_PROPERTY_H

#include "imports/avs.h"

struct property *boot_property_load(const char *filename);
struct property *boot_property_load_avs(const char *filename);
struct property *boot_property_load_cstring(const char *cstring);
void boot_property_free(struct property *prop);

#endif
