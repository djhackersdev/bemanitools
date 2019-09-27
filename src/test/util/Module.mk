testexes            += util-net-test

srcdir_util-net-test := src/test/util

ldflags_util-net-test   := \
    -lws2_32 \
    -liphlpapi \

libs_util-net-test    := \
    test \
    util \

src_util-net-test     := \
    util-net-test.c \

################################################################################
