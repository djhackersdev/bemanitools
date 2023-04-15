avsdlls         += iidxhook6

ldflags_iidxhook6   := \
    -liphlpapi \

deplibs_iidxhook6   := \
    avs \

libs_iidxhook6      := \
    iidxhook-d3d9 \
    iidxhook-util \
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
    cconfig \
    util \
    ezusb \

src_iidxhook6       := \
    dllmain.c \
