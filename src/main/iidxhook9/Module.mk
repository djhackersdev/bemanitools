avsdlls             += iidxhook9

ldflags_iidxhook9   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \

deplibs_iidxhook9   := \
    avs \

libs_iidxhook9      := \
    iidxhook-util \
    acioemu \
    asio \
    bio2emu \
    bio2emu-iidx \
    camhook \
    d3d9exhook \
    dinput \
    iidxio \
    hook \
    hooklib \
    cconfig \
    util \
    eamio \

src_iidxhook9       := \
    config-io.c \
    fs-hook.c \
    dllmain.c \
