dlls                += iidxio-ezusb2

ldflags_iidxio-ezusb2      := \
    -lwinmm \
    -lsetupapi \

libs_iidxio-ezusb2         := \
    core \
    ezusb2 \
    ezusb \
    ezusb2-iidx \
    util \
    iface \
    module \
    iface-io \
    iface-core \

src_iidxio-ezusb2          := \
    iidxio.c \
