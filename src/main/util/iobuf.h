#ifndef UTIL_IOBUF_H
#define UTIL_IOBUF_H

#include <stddef.h>
#include <stdint.h>

#include "hook/iobuf.h"

void iobuf_log(struct iobuf *buffer, const char *tag);

void iobuf_log_const(struct const_iobuf *buffer, const char *tag);

#endif
