avsdlls         += iidxhook2

ldflags_iidxhook2   := \
    -lws2_32 \
    -liphlpapi \
    -lpsapi \
    -lshlwapi \

libs_iidxhook2      := \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb-iidx-emu \
    security \
    acioemu \
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

src_iidxhook2       := \
    dllmain.c \
    iidxhook2.c \
