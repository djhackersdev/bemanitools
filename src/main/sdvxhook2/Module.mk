avsdlls             += sdvxhook2

ldflags_sdvxhook2   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \
    -lws2_32 \

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
    util \
    iface \
    iface-io \
    iface-core \
    module \
    security \

src_sdvxhook2       := \
    acio.c \
    bi2a.c \
    config-io.c \
    dllmain.c \
    nvapi.c \
    power.c \
    sdvxhook2.c \
