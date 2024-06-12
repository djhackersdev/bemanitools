avsdlls += jbhook2

deplibs_jbhook2    := \
    avs \
    glhelper \

ldflags_jbhook2   := \
    -lws2_32 \
    -liphlpapi \
    -lopengl32 \

avslibs_jbhook2 := \
    avs-ext \

libs_jbhook2       := \
    core \
    acioemu \
    jbhook-util \
    jbhook-util-p3io \
    p3ioemu \
    p3io \
    p4ioemu \
    hook \
    hooklib \
    security \
    util \
    iface \
    iface-io \
    iface-core \
    module \

src_jbhook2        := \
    dllmain.c \
    options.c
