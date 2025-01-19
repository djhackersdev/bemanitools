dlls                += iidxio-perf

ldflags_iidxio-perf      := \
    -lws2_32

libs_iidxio-perf         := \
    hdr-histogram \
    util \

src_iidxio-perf          := \
    iidxio.c \
