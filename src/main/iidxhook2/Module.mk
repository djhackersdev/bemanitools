avsdlls         += iidxhook2

ldflags_iidxhook2   := \
    -lws2_32 \
    -liphlpapi \

libs_iidxhook2      := \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-emu \
    security \
    eamio \
    acioemu \
    hook \
    hooklib \
    iidxio \
    cconfig \
    util \
    ezusb \

src_iidxhook2       := \
    config-iidxhook2.c \
    dllmain.c \
