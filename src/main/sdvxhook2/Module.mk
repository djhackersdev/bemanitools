avsdlls             += sdvxhook2

ldflags_sdvxhook2   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \

deplibs_sdvxhook2   := \
    avs \

avslibs_sdvxhook2 := \
    avs-ext \

libs_sdvxhook2      := \
    core \
    acioemu \
    bio2emu \
    camhook \
    d3d9exhook \
    hook \
    hooklib \
    cconfig \
    util \
    iface \
    iface-io \
    iface-core \
    module \

src_sdvxhook2       := \
    acio.c \
    bi2a.c \
    dllmain.c \
    config-io.c \
    nvapi.c \
    power.c \
