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
    d3d9.c \
    config-gfx.c \
    config-io.c \
