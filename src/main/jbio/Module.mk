dlls                += jbio

ldflags_jbio      := \
    -lwinmm

libs_jbio         := \
    iface \
    module \
    iface-io \
    iface-core \
    util \
    
src_jbio          := \
    jbio.c \

