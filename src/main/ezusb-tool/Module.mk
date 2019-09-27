exes            += ezusb-tool

ldflags_ezusb-tool   := \
    -lsetupapi \

libs_ezusb-tool     := \
    ezusb \
    util \

src_ezusb-tool      := \
    main.c \
