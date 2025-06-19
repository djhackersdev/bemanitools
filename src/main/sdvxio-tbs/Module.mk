dlls                += sdvxio-tbs

ldflags_sdvxio-tbs      := \
    -lsetupapi \

libs_sdvxio-tbs         := \
    cconfig \
    util \

src_sdvxio-tbs          := \
    sdvxio.c \
    load_aio.c \
