exes            += inject
imps            += dwarfstack

ldflags_inject  := \
    -mconsole \
    -lpsapi \
    -ldbghelp \

libs_inject     := \
    util \
    dwarfstack \

src_inject      := \
    main.c \
    debugger.c \
    logger.c \
    options.c \
    version.c \

volatile_inject := \
    version.c \

