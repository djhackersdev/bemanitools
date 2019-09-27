#ifndef GENINPUT_MAPPER_S11N_H
#define GENINPUT_MAPPER_S11N_H

#include <stdio.h>

#include "geninput/mapper.h"

struct mapper *mapper_impl_config_load(FILE *f);
void mapper_impl_config_save(struct mapper *m, FILE *f);

#endif
