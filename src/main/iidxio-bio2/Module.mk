dlls                += iidxio-bio2

ldflags_iidxio-bio2      := \
    -lsetupapi \
    -lws2_32 \

libs_iidxio-bio2         := \
    core \
    aciodrv \
    bio2drv \
    util \
    iface \
    module \
    iface-io \
    iface-core \
    security \

src_iidxio-bio2          := \
    iidxio.c \
