exes            += ezusb2-tool

ldflags_ezusb2-tool   := \
    -lsetupapi \

libs_ezusb2-tool     := \
    ezusb2 \
    ezusb \
    util \

src_ezusb2-tool      := \
    main.c \
