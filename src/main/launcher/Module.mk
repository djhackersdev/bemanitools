avsexes         += launcher
rc_launcher     := launcher.rc

ldflags_launcher := \
    -mconsole \

deplibs_launcher := \
    avs \
    avs-ea3 \

libs_launcher   := \
    hook \
    util \

src_launcher    := \
    avs-context.c \
    bootstrap-config.c \
    bootstrap-context.c \
    ea3-ident.c \
    eamuse.c \
    logger.c \
    main.c \
    module.c \
    options.c \
    property.c \
    stubs.c \
    version.c \

volatile_launcher := \
    version.c \

