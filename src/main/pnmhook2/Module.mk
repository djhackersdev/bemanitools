avsdlls         += pnmhook2

ldflags_pnmhook2   := \
    -liphlpapi \

deplibs_pnmhook2   := \
    avs \

libs_pnmhook2      := \
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

src_pnmhook2      := \
    acio.c \
    dllmain.c \
