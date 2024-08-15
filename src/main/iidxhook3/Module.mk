avsdlls         += iidxhook3

ldflags_iidxhook3   := \
    -lws2_32 \
    -liphlpapi \
    -lshlwapi \

deplibs_iidxhook3   := \
    avs \

libs_iidxhook3      := \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
    acioemu \
    hook \
    hooklib \
    cconfig \
    ezusb \
    iface \
    iface-io \
    iface-core \
    module \
    util \
    mxml \
    sdk-hook \

src_iidxhook3       := \
    avs-boot.c \
    dllmain.c \
    iidxhook3.c \
