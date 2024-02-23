exes            += inject

ldflags_inject  := \
    -mconsole \
    -lpsapi \

libs_inject     := \
    core \
    util \

src_inject      := \
    main.c \
    debugger.c \
    options.c \
    version.c \

volatile_inject := \
    version.c \

