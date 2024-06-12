dlls		    += iidxhook1

ldflags_iidxhook1   := \
    -lws2_32 \
    -liphlpapi \
    -lpsapi \

libs_iidxhook1      := \
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

src_iidxhook1       := \
    config-iidxhook1.c \
    dllmain.c \
    ezusb-mon.c \
    log-ezusb.c \
