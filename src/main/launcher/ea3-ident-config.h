#ifndef LAUNCHER_EA3_IDENT_CONFIG_H
#define LAUNCHER_EA3_IDENT_CONFIG_H

#include "imports/avs.h"

/* N.B. even though this might look like a Konami ABI, this is purely an
   internal data structure. */

struct ea3_ident_config {
    /* psmapped structure offset can't be zero for some stupid reason */

    uint32_t dummy;

    /* Initialized from ea3-config.xml, then fed back from sidcode_short */

    char model[4];
    char dest[4];
    char spec[4];
    char rev[4];
    char ext[11];

    /* Initialized from ea3-config.xml (hardware_id defaults to MAC addr) */

    char softid[24];
    char hardid[24];
    char pcbid[24];
};

void ea3_ident_config_init(struct ea3_ident_config *config);
void ea3_ident_config_from_file_load(
    const char *path, struct ea3_ident_config *config);
void ea3_ident_config_load(
    struct property *property, struct ea3_ident_config *config);
bool ea3_ident_config_hardid_is_defined(struct ea3_ident_config *config);
void ea3_ident_config_hardid_from_ethernet_set(struct ea3_ident_config *config);

#endif
