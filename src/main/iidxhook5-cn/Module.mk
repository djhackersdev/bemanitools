avsdlls         += iidxhook5-cn

ldflags_iidxhook5-cn   := \
    -liphlpapi \

deplibs_iidxhook5-cn   := \
    avs \

libs_iidxhook5-cn      := \
    iidxhook-util \
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
    cconfig \
    util \
    ezusb \

src_iidxhook5-cn       := \
    avs-boot.c \
    path.c \
    dllmain.c \
