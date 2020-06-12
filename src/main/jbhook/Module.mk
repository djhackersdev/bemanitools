avsdlls += jbhook

deplibs_jbhook    := \
    avs \

ldflags_jbhook   := \
    -lws2_32 \
    -liphlpapi \

libs_jbhook       := \
    acioemu \
    eamio \
    jbio \
    p4ioemu \
    hook \
    hooklib \
    util \

src_jbhook        := \
    acio.c \
    dllmain.c \
    eamuse.c \
    gfx.c \
    options.c \
    io.c
