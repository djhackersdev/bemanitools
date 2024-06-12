exes                  += vigem-ddrio

deplibs_vigem-ddrio  := \
    ViGEmClient \

cppflags_vigem-ddrio  := \
    -I src/imports \

ldflags_vigem-ddrio  := \
    -lsetupapi \

libs_vigem-ddrio     := \
    core \
    cconfig \
    util \
    vigemstub \
    module \
    iface-io \
    iface-core \

src_vigem-ddrio      := \
    main.c \
    config-vigem-ddrio.c \
