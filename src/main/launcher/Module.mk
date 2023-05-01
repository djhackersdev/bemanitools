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
    bs-config.c \
    ea3-config.c \
    main.c \
    module.c \
    options.c \
    property.c \
    stubs.c \
    version.c \

volatile_launcher := \
    version.c \

