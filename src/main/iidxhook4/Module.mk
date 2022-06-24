avsdlls         += iidxhook4

ldflags_iidxhook4   := \
    -liphlpapi \

deplibs_iidxhook4   := \
    avs \

libs_iidxhook4      := \
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

src_iidxhook4       := \
    dllmain.c \
