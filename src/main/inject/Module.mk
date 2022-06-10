exes            += inject

ldflags_inject  := \
    -mconsole \
    -lpsapi \

libs_inject     := \
    util \

src_inject      := \
    main.c \
    debugger.c \
    logger.c \
    options.c \
    version.c \
