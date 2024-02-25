exes            += ezusb-tool

ldflags_ezusb-tool   := \
    -lsetupapi \

libs_ezusb-tool     := \
    core \
    ezusb \
    util \

src_ezusb-tool      := \
    main.c \
