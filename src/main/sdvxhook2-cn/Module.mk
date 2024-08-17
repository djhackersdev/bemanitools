avsdlls             += sdvxhook2-cn

ldflags_sdvxhook2-cn   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \
    -lws2_32 \

deplibs_sdvxhook2-cn   := \
    avs \

avslibs_sdvxhook2-cn:= \
    avs-ext \

libs_sdvxhook2-cn      := \
    core \
    acioemu \
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
    security \

src_sdvxhook2-cn       := \
    acio.c \
    kfca.c \
    dllmain.c \
    config-cn.c \
    unis-version.c \
