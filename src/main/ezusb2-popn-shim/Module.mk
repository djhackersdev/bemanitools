dlls            += ezusb2-popn-shim

ldflags_ezusb2-popn-shim   := \
    -lsetupapi \

libs_ezusb2-popn-shim      := \
    ezusb2-emu \
    hook \
    hooklib \
    util \

src_ezusb2-popn-shim       := \
    dllmain.c \
    proxy.c \
