dlls                += sdvxio-valk

ldflags_sdvxio-valk      := \
    -lsetupapi \

libs_sdvxio-valk         := \
    cconfig \
    util \

src_sdvxio-valk          := \
    sdvxio.c \
    load_aio.c \
