exes                  += vigem-sdvxio

deplibs_vigem-sdvxio  := \
    ViGEmClient \

cppflags_vigem-sdvxio  := \
    -I src/imports \

ldflags_vigem-sdvxio  := \
    -lsetupapi \

libs_vigem-sdvxio     := \
    core \
    cconfig \
    sdvxio \
    util \
    vigemstub \

src_vigem-sdvxio      := \
    main.c \
    config-vigem-sdvxio.c \
