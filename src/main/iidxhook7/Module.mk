avsdlls         += iidxhook7

ldflags_iidxhook7   := \
    -liphlpapi \
    -ld3d9 \

deplibs_iidxhook7   := \
    avs \

libs_iidxhook7      := \
    iidxhook-d3d9 \
    iidxhook-util \
    cconfig \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
    eamio \
    acioemu \
    hook \
    hooklib \
    iidxio \
    util \
    ezusb \

src_iidxhook7       := \
    dllmain.c \
