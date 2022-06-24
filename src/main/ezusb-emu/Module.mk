libs            += ezusb-emu

libs_ezusb-emu      := \
    ezusb \

src_ezusb-emu     := \
    desc.c \
    device.c \
    util.c \
    node-coin.c \
    node-eeprom.c \
    node-none.c \
    node-security-mem.c \
    node-security-plug.c \
    node-sram.c \
    node-wdt.c \
    nodes.c
