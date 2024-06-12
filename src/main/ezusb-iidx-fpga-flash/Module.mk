exes            += ezusb-iidx-fpga-flash

ldflags_ezusb-iidx-fpga-flash   := \
    -lsetupapi \

libs_ezusb-iidx-fpga-flash     := \
    core \
    ezusb \
    ezusb-iidx \
    util \
    iface-core \

src_ezusb-iidx-fpga-flash      := \
    main.c \
