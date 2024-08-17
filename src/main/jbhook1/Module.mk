avsdlls += jbhook1
imps += glhelper

deplibs_jbhook1	:= \
    avs \
    glhelper \

ldflags_jbhook1   := \
    -lws2_32 \
    -liphlpapi \
    -lopengl32 \
    -lshlwapi \

libs_jbhook1       := \
    core \
    acioemu \
    jbhook-util \
    jbhook-util-p3io \
    p3ioemu \
    p3io \
    hook \
    hooklib \
    security \
    util \
    iface \
    iface-io \
    iface-core \
    module \
    sdk-hook \
    mxml \

src_jbhook1        := \
    avs-boot.c \
    config-gfx.c \
    config-eamuse.c \
    config-security.c \
    dllmain.c \
    gfx.c \
    jbhook1.c \
    log-gftools.c \
