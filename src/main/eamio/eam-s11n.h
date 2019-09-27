#ifndef GENINPUT_EAM_S11N_H
#define GENINPUT_EAM_S11N_H

#include <stdio.h>

#include "eamio/eam-impl.h"

struct eam *eam_impl_config_load(FILE *f);
void eam_impl_config_save(struct eam *eam, FILE *f);

#endif
