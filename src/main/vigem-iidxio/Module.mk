exes                  += vigem-iidxio

deplibs_vigem-iidxio  := \
    ViGEmClient \

cppflags_vigem-iidxio  := \
    -I src/imports \

ldflags_vigem-iidxio  := \
    -lsetupapi \
    -lws2_32 \

libs_vigem-iidxio     := \
    core \
    vigemstub \
    module \
    ezusb \
    iface \
    iface-io \
    iface-core \
    security \
    util \

src_vigem-iidxio      := \
    cab-16seg-sequencer.c \
    cab-light-sequencer.c \
    config.c \
    main.c \
