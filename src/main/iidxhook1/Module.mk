dlls		    += iidxhook1

ldflags_iidxhook1   := \
    -lws2_32 \
    -liphlpapi \
    -lpsapi \
    -lshlwapi \

libs_iidxhook1      := \
    sdk-hook \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb-iidx-emu \
    security \
    acioemu \
    hook \
    hooklib \
    cconfig \
    module \
    ezusb \
    iface \
    iface-io \
    iface-core \
    util \
    mxml \

src_iidxhook1       := \
    dllmain.c \
    iidxhook1.c \
    ezusb-mon.c \
    log-ezusb.c \
