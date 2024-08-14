avsdlls         += iidxhook4-cn

ldflags_iidxhook4-cn   := \
    -liphlpapi \
    -lws2_32 \

deplibs_iidxhook4-cn   := \
    avs \

libs_iidxhook4-cn      := \
	avs-util \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    security \
    hook \
    hooklib \
    iidxio \
    cconfig \
    util \
    ezusb \

src_iidxhook4-cn       := \
    avs-boot.c \
    path.c \
    dllmain.c \
