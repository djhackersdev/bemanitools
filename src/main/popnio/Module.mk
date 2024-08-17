dlls                += popnio

ldflags_popnio      := \
    -lwinmm

libs_popnio         := \
    iface \
    module \
    iface-io \
    iface-core \
    util \

src_popnio          := \
    popnio.c \
