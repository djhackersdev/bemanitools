dlls                += vefxio

ldflags_vefxio      := \
    -lwinmm

libs_vefxio         := \
    iface-core \
    geninput \

src_vefxio          := \
    vefxio.c \
