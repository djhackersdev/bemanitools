dlls += sdvxio-bio2

ldflags_sdvxio-bio2   := \
    -lsetupapi \
    -lws2_32 \

libs_sdvxio-bio2 := \
    aciodrv \
    bio2drv \
    util \
    iface-core \
    util \
    security \
    core \

src_sdvxio-bio2	:= \
    sdvxio.c \

