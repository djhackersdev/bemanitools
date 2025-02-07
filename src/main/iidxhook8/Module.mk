avsdlls             += iidxhook8

ldflags_iidxhook8   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \
    -ld3d9 \
    -ldwmapi\
    -lgdi32 \

deplibs_iidxhook8   := \
    avs \

libs_iidxhook8      := \
    iidxhook-d3d9 \
    iidxhook-util \
    acioemu \
    bio2emu \
    bio2emu-iidx \
    camhook \
    iidxio \
    hook \
    hooklib \
    cconfig \
    util \
    eamio \
    imgui-bt \
    imgui-debug \
    imgui \

src_iidxhook8       := \
    config-io.c \
    dllmain.c \
