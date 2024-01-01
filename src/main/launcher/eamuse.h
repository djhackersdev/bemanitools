#ifndef LAUNCHER_EAMUSE_H
#define LAUNCHER_EAMUSE_H

#include "bootstrap-config.h"
#include "ea3-ident.h"
#include "options.h"

void eamuse_init(const struct bootstrap_eamuse_config* config,
    const struct ea3_ident* ea3_ident,
    const struct options* options);

void eamuse_fini();

#endif