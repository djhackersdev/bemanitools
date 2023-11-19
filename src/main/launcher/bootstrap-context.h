#ifndef LAUNCHER_BOOTSTRAP_CONTEXT_H
#define LAUNCHER_BOOTSTRAP_CONTEXT_H

#include <stdlib.h>

#include "launcher/bs-config.h"

void bootstrap_context_init();
void bootstrap_context_finit();

void bootstrap_context_init(
    const char *avs_config_path,
    const char *ea3_config_path,
    size_t std_heap_size,
    size_t avs_heap_size,
    const char *logfile,
    const char *module,
    struct bootstrap_config *config);

void bootstrap_context_init_from_file(
    const char *config_path,
    const char *selector,
    struct bootstrap_config *config);

void bootstrap_context_post_avs_setup(const struct bootstrap_config *config);

#endif