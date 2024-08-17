#include "util/defs.h"

const char *launcher_build_date = __DATE__ " " __TIME__;
const char *launcher_gitrev = STRINGIFY(GITREV);
const char *launcher_linked_avs_version = STRINGIFY(AVS_VERSION);