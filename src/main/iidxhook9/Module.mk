avsdlls             += iidxhook9

ldflags_iidxhook9   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \
    -lws2_32 \

deplibs_iidxhook9   := \
    avs \

avslibs_iidxhook9 := \
    avs-ext \

libs_iidxhook9      := \
    core \
    iidxhook-util \
    acioemu \
    asio \
    bio2emu \
    bio2emu-iidx \
    camhook \
    d3d9exhook \
    dinput \
    hook \
    hooklib \
    cconfig \
    iface \
    iface-io \
    iface-core \
    module \
    util \

src_iidxhook9       := \
    config-io.c \
    fs-hook.c \
    dllmain.c \
