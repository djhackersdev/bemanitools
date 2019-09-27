avsdlls         += iidxhook7

ldflags_iidxhook7   := \
    -liphlpapi \

deplibs_iidxhook7   := \
    avs \

libs_iidxhook7      := \
    iidxhook-util \
    cconfig \
    ezusb-emu \
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
