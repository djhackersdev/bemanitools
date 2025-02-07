avsdlls         += iidxhook2

ldflags_iidxhook2   := \
    -lws2_32 \
    -liphlpapi \
    -ld3d9 \
    -ldwmapi\
    -lgdi32 \

libs_iidxhook2      := \
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

src_iidxhook2       := \
    config-iidxhook2.c \
    dllmain.c \
