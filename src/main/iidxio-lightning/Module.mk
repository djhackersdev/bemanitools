dlls                += iidxio-lightning

ldflags_iidxio-lightning      := \
    -lsetupapi \

libs_iidxio-lightning         := \
    cconfig \
    util \

src_iidxio-lightning          := \
    iidxio.c \
    load_aio.c \
