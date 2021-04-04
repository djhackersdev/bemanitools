dlls                += jbio-p4io

ldflags_jbio-p4io      := \
    -lsetupapi

src_jbio-p4io          := \
    config-h44b.c \
    h44b.c \
    jbio.c \

libs_jbio-p4io      := \
    aciodrv \
    aciomgr \
    cconfig \
    p4iodrv \
    util \
