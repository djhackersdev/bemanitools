avsdlls             += iidxhook8

ldflags_iidxhook8   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \

deplibs_iidxhook8   := \
    avs \

libs_iidxhook8      := \
    iidxhook-util \
    acioemu \
    bio2emu \
    camhook \
    iidxio \
    hook \
    hooklib \
    cconfig \
    util \
    eamio \
    
src_iidxhook8       := \
    bi2a.c \
    config-io.c \
    dllmain.c \
