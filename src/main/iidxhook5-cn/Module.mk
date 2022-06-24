avsdlls         += iidxhook5-cn

ldflags_iidxhook5-cn   := \
    -liphlpapi \
    -lws2_32 \

deplibs_iidxhook5-cn   := \
    avs \

libs_iidxhook5-cn      := \
    iidxhook-util \
    eamio \
    ezusb-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
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
