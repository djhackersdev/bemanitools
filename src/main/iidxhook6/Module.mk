avsdlls         += iidxhook6

ldflags_iidxhook6   := \
    -liphlpapi \

deplibs_iidxhook6   := \
    avs \

avslibs_iidxhook6 := \
    avs-ext \

libs_iidxhook6      := \
    core \
    iidxhook-d3d9 \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
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

src_iidxhook6       := \
    dllmain.c \
