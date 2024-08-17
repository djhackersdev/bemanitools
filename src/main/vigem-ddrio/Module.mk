exes                  += vigem-ddrio

deplibs_vigem-ddrio  := \
    ViGEmClient \

cppflags_vigem-ddrio  := \
    -I src/imports \

ldflags_vigem-ddrio  := \
    -lsetupapi \
    -lws2_32 \

libs_vigem-ddrio     := \
    core \
    vigemstub \
    module \
    iface-io \
    iface-core \
    security \
    util \

src_vigem-ddrio      := \
    config.c \
    main.c \
