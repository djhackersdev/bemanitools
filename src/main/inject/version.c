#include "util/defs.h"

const char *inject_build_date = __DATE__ " " __TIME__;
const char *inject_gitrev = STRINGIFY(GITREV);