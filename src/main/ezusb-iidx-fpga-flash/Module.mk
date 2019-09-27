exes            += ezusb-iidx-fpga-flash

ldflags_ezusb-iidx-fpga-flash   := \
    -lsetupapi \

libs_ezusb-iidx-fpga-flash     := \
    ezusb \
    ezusb-iidx \
    util \

src_ezusb-iidx-fpga-flash      := \
    main.c \
