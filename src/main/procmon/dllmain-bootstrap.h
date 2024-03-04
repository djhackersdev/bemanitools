#ifndef DLLMAIN_BOOTSTRAP_H
#define DLLMAIN_BOOTSTRAP_H

#include <stdint.h>

#include "imports/avs.h"

void btsdk_dllmain_bootstrap_process_attach();

void btsdk_dllmain_bootstrap_process_detach();

#endif