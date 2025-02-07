avsdlls         += iidxhook5

ldflags_iidxhook5   := \
    -liphlpapi \
    -ld3d9 \
    -ldwmapi\
    -lgdi32 \

deplibs_iidxhook5   := \
    avs \

libs_iidxhook5      := \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
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

src_iidxhook5       := \
    dllmain.c \
    ifs-snd-redir.c \
