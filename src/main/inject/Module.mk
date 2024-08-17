exes            += inject
imps            += dwarfstack

ldflags_inject  := \
    -mconsole \
    -lpsapi \
    -ldbghelp \

libs_inject     := \
    core \
    util \
    dwarfstack \
    iface-core \
    mxml \

src_inject      := \
    debug-config.c \
    debugger-config.c \
    debugger.c \
    hooks-config.c \
    inject-config.c \
    inject.c \
    logger-config.c \
    logger.c \
    main.c \
    options.c \
    version.c \

volatile_inject := \
    version.c \

