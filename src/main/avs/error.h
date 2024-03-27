#ifndef AVS_ERROR_H
#define AVS_ERROR_H

#include "imports/avs.h"

const char *avs_error_str(avs_error error);

const char *avs_error_property_error_get_and_clear(struct property *prop);

#endif
