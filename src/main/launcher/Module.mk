avsexes         += launcher
rc_launcher     := launcher.rc

ldflags_launcher := \
    -mconsole \
    -ldbghelp \
    -lpsapi \

deplibs_launcher := \
    avs \
    avs-ea3 \

avslibs_launcher := \
    avs-ext \

libs_launcher   := \
    hook \
    util \
    iface-core \
    iface \
    module \
    core \
    mxml \

src_launcher    := \
    app.c \
    avs-config.c \
    avs.c \
    bootstrap-config.c \
    bootstrap.c \
    debug.c \
    ea3-ident-config.c \
    eamuse-config.c \
    eamuse.c \
    hooks.c \
    launcher-config.c \
    launcher.c \
    main.c \
    options.c \
    stubs.c \
    version.c \

volatile_launcher := \
    version.c \

