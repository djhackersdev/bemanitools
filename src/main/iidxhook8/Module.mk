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
	avs-util \
    core \
    iidxhook-d3d9 \
    iidxhook-util \
    acioemu \
    bio2emu \
    bio2emu-iidx \
    camhook \
    iidxio \
    hook \
    hooklib \
    cconfig \
    util \
    eamio \
    
src_iidxhook8       := \
    config-io.c \
    dllmain.c \
