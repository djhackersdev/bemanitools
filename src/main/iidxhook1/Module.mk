dlls		    += iidxhook1

ldflags_iidxhook1   := \
    -lws2_32 \
    -liphlpapi \
    -ld3d9 \
    -ldwmapi\
    -lgdi32 \

libs_iidxhook1      := \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
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
    imgui-bt \
    imgui-debug \
    imgui \

src_iidxhook1       := \
    config-iidxhook1.c \
    dllmain.c \
    ezusb-mon.c \
    log-ezusb.c \
