dlls                += iidxio-ezusb

ldflags_iidxio-ezusb      := \
    -lwinmm \
    -lsetupapi \

libs_iidxio-ezusb         := \
    core \
    ezusb \
    ezusb2 \
    ezusb-iidx \
    util \

src_iidxio-ezusb          := \
    iidxio.c \
