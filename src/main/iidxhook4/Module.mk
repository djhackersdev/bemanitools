avsdlls         += iidxhook4

ldflags_iidxhook4   := \
    -liphlpapi \

deplibs_iidxhook4   := \
    avs \

avslibs_iidxhook4 := \
    avs-ext \

libs_iidxhook4      := \
    core \
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

src_iidxhook4       := \
    dllmain.c \
