avsdlls			+= popnhook1

deplibs_popnhook1	:= \
    avs \

ldflags_popnhook1   := \
    -lws2_32 \
    -liphlpapi \
    -lshlwapi \

libs_popnhook1		:= \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb2-popn-emu \
    ezusb2-emu \
    security \
    acioemu \
    hook \
    hooklib \
    util \
    ezusb \
    security \
    popnhook-util \
    iface \
    iface-io \
    iface-core \
    module \
    sdk-hook \
    mxml \

src_popnhook1		:= \
    avs-boot.c \
    config-eamuse.c \
    config-gfx.c \
    config-sec.c \
    d3d9.c \
    dllmain.c \
    filesystem.c \
    popnhook1.c \
