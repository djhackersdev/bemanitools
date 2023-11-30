#ifndef LAUNCHER_EA3_IDENT_H
#define LAUNCHER_EA3_IDENT_H

#include "imports/avs.h"

#include "launcher/module.h"

/* N.B. even though this might look like a Konami ABI, this is purely an
   internal data structure. */

struct ea3_ident {
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

void ea3_ident_init(struct ea3_ident *ident);
void ea3_ident_initialize_from_file(const char *path, struct ea3_ident *ea3_ident);
bool ea3_ident_from_property(
    struct ea3_ident *ident, struct property *ea3_config);
void ea3_ident_hardid_from_ethernet(struct ea3_ident *ident);
void ea3_ident_to_property(
    const struct ea3_ident *ident, struct property *ea3_config);
    
#endif
