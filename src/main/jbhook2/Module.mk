avsdlls += jbhook2

deplibs_jbhook2    := \
    avs \
    glhelper \

ldflags_jbhook2   := \
    -lws2_32 \
    -liphlpapi \
    -lopengl32 \

libs_jbhook2       := \
    avs-util \
    core \
    acioemu \
    eamio \
    jbio \
    jbhook-util \
    jbhook-util-p3io \
    p3ioemu \
    p3io \
    p4ioemu \
    hook \
    hooklib \
    security \
    util \

src_jbhook2        := \
    dllmain.c \
    options.c
