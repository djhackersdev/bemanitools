exes                  += vigem-iidxio

deplibs_vigem-iidxio  := \
    ViGEmClient \

cppflags_vigem-iidxio  := \
    -I src/imports \

ldflags_vigem-iidxio  := \
    -lsetupapi \

libs_vigem-iidxio     := \
    cconfig \
    iidxio \
    util \
    vigemstub \

src_vigem-iidxio      := \
    cab-16seg-sequencer.c \
    cab-light-sequencer.c \
    main.c \
    config.c \
