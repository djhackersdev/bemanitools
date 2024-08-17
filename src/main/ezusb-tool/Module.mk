exes            += ezusb-tool

ldflags_ezusb-tool   := \
    -lsetupapi \

libs_ezusb-tool     := \
    core \
    ezusb \
    util \
    iface-core \

src_ezusb-tool      := \
    main.c \
