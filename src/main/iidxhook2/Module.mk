avsdlls         += iidxhook2

ldflags_iidxhook2   := \
    -lws2_32 \
    -liphlpapi \
    -lpsapi \

libs_iidxhook2      := \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb-iidx-emu \
    security \
    acioemu \
    hook \
    hooklib \
    cconfig \
    ezusb \
    iface \
    iface-io \
    iface-core \
    module \
    util \
    mxml \

src_iidxhook2       := \
    config-iidxhook2.c \
    dllmain.c \
