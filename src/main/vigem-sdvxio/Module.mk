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
    util \
    vigemstub \
    module \
    iface \
    iface-io \
    iface-core \

src_vigem-sdvxio      := \
    main.c \
    config-vigem-sdvxio.c \
