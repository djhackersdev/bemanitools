avsdlls         += iidxhook3

ldflags_iidxhook3   := \
    -lws2_32 \
    -liphlpapi \

deplibs_iidxhook3   := \
    avs \

libs_iidxhook3      := \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
    acioemu \
    eamio \
    hook \
    hooklib \
    iidxio \
    cconfig \
    util \
    ezusb \

src_iidxhook3       := \
    avs-boot.c \
    dllmain.c \
