dlls            += ezusb2-popn-shim

ldflags_ezusb2-popn-shim   := \
    -lsetupapi \

libs_ezusb2-popn-shim      := \
    core \
    ezusb2-emu \
    hook \
    hooklib \
    util \
    iface-core \

src_ezusb2-popn-shim       := \
    dllmain.c \
    proxy.c \
