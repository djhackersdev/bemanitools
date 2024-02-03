avsexes         += launcher
rc_launcher     := launcher.rc

ldflags_launcher := \
    -mconsole \
    -ldbghelp \

deplibs_launcher := \
    avs \
    avs-ea3 \

libs_launcher   := \
    avs-util \
    hook \
    util \
    dwarfstack \
    procmon-lib \

src_launcher    := \
    avs-config.c \
    avs.c \
    bootstrap-config.c \
    bootstrap.c \
    debug.c \
    ea3-ident-config.c \
    eamuse-config.c \
    eamuse.c \
    hook.c \
    launcher-config.c \
    launcher.c \
    logger.c \
    main.c \
    module.c \
    options.c \
    property-util.c \
    stubs.c \
    version.c \

volatile_launcher := \
    version.c \

