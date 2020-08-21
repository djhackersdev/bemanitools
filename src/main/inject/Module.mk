exes            += inject

ldflags_inject  := \
    -mconsole \
    -lpsapi \

libs_inject     := \
    hook \
    hooklib \
    util \

src_inject      := \
    debugger.c \
    logger.c \
    main.c \
    options.c \
    version.c \
