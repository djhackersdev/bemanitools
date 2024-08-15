avsdlls += jbhook3

deplibs_jbhook3    := \
    avs \

ldflags_jbhook3   := \
    -lws2_32 \
    -liphlpapi \

avslibs_jbhook3 := \
    avs-ext \

libs_jbhook3       := \
    core \
    acioemu \
    jbhook-util \
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

src_jbhook3        := \
    dllmain.c \
	gfx.c \
    options.c
