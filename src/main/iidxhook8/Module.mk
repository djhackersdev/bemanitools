avsdlls             += iidxhook8

ldflags_iidxhook8   := \
    -liphlpapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lole32 \
    -lws2_32 \

deplibs_iidxhook8   := \
    avs \

avslibs_iidxhook8 := \
    avs-ext \

libs_iidxhook8      := \
    core \
    iidxhook-d3d9 \
    iidxhook-util \
    acioemu \
    bio2emu \
    bio2emu-iidx \
    camhook \
    hook \
    hooklib \
    cconfig \
    iface \
    iface-io \
    iface-core \
    module \
    util \
    security \
    
src_iidxhook8       := \
    config-io.c \
    dllmain.c \
    iidxhook8.c \
