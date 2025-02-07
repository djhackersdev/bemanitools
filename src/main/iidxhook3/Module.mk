avsdlls         += iidxhook3

ldflags_iidxhook3   := \
    -lws2_32 \
    -liphlpapi \
    -ld3d9 \
    -ldwmapi\
    -lgdi32 \

libs_iidxhook3      := \
    iidxhook-util \
    iidxhook-d3d9 \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
    acioemu \
    eamio \
    hook \
    hooklib \
    iidxio \
    cconfig \
    util \
    ezusb \
    imgui-bt \
    imgui-debug \
    imgui \

src_iidxhook3       := \
    dllmain.c \
