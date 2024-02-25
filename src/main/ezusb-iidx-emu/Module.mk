libs            += ezusb-iidx-emu

libs_ezusb-iidx-emu      := \
    core \
    ezusb-emu \
    ezusb-iidx-16seg-emu \

src_ezusb-iidx-emu     := \
    card-mag.c \
    msg.c \
    node-fpga.c \
    node-serial.c \
    nodes.c \
