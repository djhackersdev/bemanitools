dlls                += jbio-p4io

ldflags_jbio-p4io      := \
    -lsetupapi \
    -lws2_32 \

libs_jbio-p4io      := \
    core \
    aciodrv \
    p4iodrv \
    util \
    iface \
    module \
    iface-core \
    iface-acio \
    security \

src_jbio-p4io          := \
    config-h44b.c \
    h44b.c \
    jbio.c \
