avsdlls         += iidxhook7

ldflags_iidxhook7   := \
    -liphlpapi \
    -ld3d9 \

deplibs_iidxhook7   := \
    avs \

avslibs_iidxhook7 := \
    avs-ext \

libs_iidxhook7      := \
    core \
    iidxhook-d3d9 \
    iidxhook-util \
    cconfig \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
    acioemu \
    hook \
    hooklib \
    ezusb \
    iface \
    iface-io \
    iface-core \
    module \
    util \

src_iidxhook7       := \
    dllmain.c \
    iidxhook7.c \
