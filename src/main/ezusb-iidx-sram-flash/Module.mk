exes            += ezusb-iidx-sram-flash

ldflags_ezusb-iidx-sram-flash   := \
    -lsetupapi \

libs_ezusb-iidx-sram-flash     := \
    core \
    ezusb \
    ezusb-iidx \
    util \
    iface-core \

src_ezusb-iidx-sram-flash      := \
    main.c \
