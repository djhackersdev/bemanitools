avsdlls += jbhook3

deplibs_jbhook3    := \
    avs \

ldflags_jbhook3   := \
    -lws2_32 \
    -liphlpapi \

libs_jbhook3       := \
    avs-util \
    core \
    acioemu \
    eamio \
    jbio \
    jbhook-util \
    p3ioemu \
    p3io \
    p4ioemu \
    hook \
    hooklib \
    security \
    util \

src_jbhook3        := \
    dllmain.c \
	gfx.c \
    options.c
