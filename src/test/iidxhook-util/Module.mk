testexes            += iidxhook-util-config-eamuse-test

srcdir_iidxhook-util-config-eamuse-test := src/test/iidxhook-util

ldflags_iidxhook-util-config-eamuse-test   := \
    -lws2_32 \

libs_iidxhook-util-config-eamuse-test     := \
    security \
    iidxhook-util \
    cconfig \
    test \
    util \

src_iidxhook-util-config-eamuse-test     := \
    iidxhook-util-config-eamuse-test.c \

################################################################################

testexes            += iidxhook-util-config-gfx-test

srcdir_iidxhook-util-config-gfx-test := src/test/iidxhook-util

libs_iidxhook-util-config-gfx-test     := \
    security \
    iidxhook-util \
    cconfig \
    test \
    util \

src_iidxhook-util-config-gfx-test     := \
    iidxhook-util-config-gfx-test.c \

################################################################################

testexes            += iidxhook-util-config-misc-test

srcdir_iidxhook-util-config-misc-test := src/test/iidxhook-util

libs_iidxhook-util-config-misc-test     := \
    security \
    iidxhook-util \
    cconfig \
    test \
    util \

src_iidxhook-util-config-misc-test     := \
    iidxhook-util-config-misc-test.c \

################################################################################

testexes            += iidxhook-util-config-sec-test

srcdir_iidxhook-util-config-sec-test := src/test/iidxhook-util

libs_iidxhook-util-config-sec-test     := \
    security \
    iidxhook-util \
    cconfig \
    test \
    util \

src_iidxhook-util-config-sec-test     := \
    iidxhook-util-config-sec-test.c \

################################################################################