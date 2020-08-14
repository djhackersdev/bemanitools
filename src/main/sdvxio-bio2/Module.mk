dlls += sdvxio-bio2

ldflags_sdvxio-bio2   := \
    -lsetupapi \

libs_sdvxio-bio2 := \
    aciodrv \
    bio2drv \
    cconfig \
    util \

src_sdvxio-bio2	:= \
    sdvxio.c \

