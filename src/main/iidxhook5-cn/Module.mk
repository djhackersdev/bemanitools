avsdlls         += iidxhook5-cn

ldflags_iidxhook5-cn   := \
    -liphlpapi \
    -lws2_32 \
    -lshlwapi \

deplibs_iidxhook5-cn   := \
    avs \

avslibs_iidxhook5-cn := \
    avs-ext \

libs_iidxhook5-cn      := \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    security \
    hook \
    hooklib \
    ezusb \
    iface \
    iface-io \
    iface-core \
    module \
    util \
    mxml \
    sdk-hook \

src_iidxhook5-cn       := \
    avs-boot.c \
    dllmain.c \
    iidxhook5-cn.c \
    path.c \
