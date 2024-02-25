#ifndef AVS_UTIL_ERROR_H
#define AVS_UTIL_ERROR_H

#include "imports/avs.h"

const char *avs_util_error_str(avs_error error);

const char *avs_util_property_error_get_and_clear(struct property *prop);

#endif
