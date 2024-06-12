#ifndef AVS_EXT_ERROR_H
#define AVS_EXT_ERROR_H

#include "imports/avs.h"

const char *avs_ext_error_str(avs_error error);

const char *avs_ext_error_property_error_get_and_clear(struct property *prop);

#endif
