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

libs_sdvxhook2      := \
    acioemu \
    bio2emu \
    camhook \
    d3d9exhook \
    sdvxio \
    hook \
    hooklib \
    cconfig \
    util \
    eamio \

src_sdvxhook2       := \
    acio.c \
    bi2a.c \
    dllmain.c \
    config-io.c \
