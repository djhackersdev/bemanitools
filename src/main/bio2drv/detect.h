#ifndef BIO2DRV_DETECT_H
#define BIO2DRV_DETECT_H

#include <stdbool.h>

#include "bemanitools/glue.h"

enum bio2drv_detect_mode {
    DETECT_DEVICEDESC = 0x1, // look for serial devices containing BIO2(VIDEO)
    DETECT_FRIENDLYNAME = 0x2, // look for serial devices containing BIO2(VIDEO)
    DETECT_DEVICEID = 0x3, // look for serial devices by vid/pid
};

void bio2drv_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

bool bio2drv_detect(
    enum bio2drv_detect_mode mode, size_t devnum, char *path, size_t length);

#endif
