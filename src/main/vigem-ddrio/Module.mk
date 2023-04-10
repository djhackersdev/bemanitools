exes                  += vigem-ddrio

deplibs_vigem-ddrio  := \
    ViGEmClient \

cppflags_vigem-ddrio  := \
    -I src/imports \

ldflags_vigem-ddrio  := \
    -lsetupapi \

libs_vigem-ddrio     := \
    cconfig \
    ddrio \
    util \
    vigemstub \

src_vigem-ddrio      := \
    main.c \
    config-vigem-ddrio.c \
