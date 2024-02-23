avsdlls             += sdvxhook2-cn

ldflags_sdvxhook2-cn   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \

deplibs_sdvxhook2-cn   := \
    avs \

libs_sdvxhook2-cn      := \
    avs-util \
    core \
    acioemu \
    camhook \
    d3d9exhook \
    sdvxio \
    hook \
    hooklib \
    cconfig \
    util \

src_sdvxhook2-cn       := \
    acio.c \
    kfca.c \
    dllmain.c \
    config-cn.c \
    unis-version.c \
