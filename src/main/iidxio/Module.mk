dlls                += iidxio

ldflags_iidxio      := \
    -lwinmm

libs_iidxio         := \
    iface \
    module \
    iface-io \
    iface-core \
    util \

src_iidxio          := \
    iidxio.c \
