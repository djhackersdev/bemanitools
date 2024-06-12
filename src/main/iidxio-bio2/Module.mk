dlls                += iidxio-bio2

ldflags_iidxio-bio2      := \
    -lsetupapi \

libs_iidxio-bio2         := \
    core \
    aciodrv \
    bio2drv \
    cconfig \
    util \
    iface \
    module \
    iface-io \
    iface-core \

src_iidxio-bio2          := \
    iidxio.c \
