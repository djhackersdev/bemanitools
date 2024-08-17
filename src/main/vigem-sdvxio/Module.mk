exes                  += vigem-sdvxio

deplibs_vigem-sdvxio  := \
    ViGEmClient \

cppflags_vigem-sdvxio  := \
    -I src/imports \

ldflags_vigem-sdvxio  := \
    -lsetupapi \
    -lws2_32 \

libs_vigem-sdvxio     := \
    core \
    vigemstub \
    module \
    iface \
    iface-io \
    iface-core \
    security \
    util \

src_vigem-sdvxio      := \
    config.c \
    main.c \
