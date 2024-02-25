testdlls		    += d3d9hook

srcdir_d3d9hook := src/test/d3d9hook

ldflags_d3d9hook   := \

libs_d3d9hook     := \
    core \
    hook \
    test \
    util \

src_d3d9hook     := \
    dllmain.c \

################################################################################

testexes            += d3d9hook-test

srcdir_d3d9hook-test := src/test/d3d9hook

ldflags_d3d9hook-test   := \
    -ld3d9

libs_d3d9hook-test     := \
    hook \
    test \
    util \
    core \

src_d3d9hook-test      := \
    main.c \

################################################################################
