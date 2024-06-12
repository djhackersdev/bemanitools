avsdlls         += iidxhook4-cn

ldflags_iidxhook4-cn   := \
    -liphlpapi \
    -lws2_32 \

deplibs_iidxhook4-cn   := \
    avs \

avslibs_iidxhook4-cn := \
    avs-ext \

libs_iidxhook4-cn      := \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    security \
    hook \
    hooklib \
    cconfig \
    util \
    ezusb \
    iface \
    iface-io \
    iface-core \
    module \
    util \

src_iidxhook4-cn       := \
    avs-boot.c \
    path.c \
    dllmain.c \
