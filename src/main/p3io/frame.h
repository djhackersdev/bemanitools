#ifndef P3IO_P3IO_H
#define P3IO_P3IO_H

#include <windows.h>

#include <stdint.h>

#include "util/iobuf.h"

HRESULT p3io_frame_encode(
        struct iobuf *dest,
        const void *bytes,
        size_t nbytes);

HRESULT p3io_frame_decode(
        struct iobuf *dest,
        struct const_iobuf *src);

#endif
