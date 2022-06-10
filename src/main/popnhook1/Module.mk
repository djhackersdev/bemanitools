avsdlls			+= popnhook1

deplibs_popnhook1	:= \
    avs \

ldflags_popnhook1   := \
    -lws2_32 \
    -liphlpapi \

libs_popnhook1		:= \
    iidxhook-util \
    ezusb-iidx-emu \
    ezusb2-popn-emu \
    ezusb2-emu \
    security \
    acioemu \
    hook \
    hooklib \
    eamio \
    popnio \
    cconfig \
    util \
    ezusb \
    security \
    popnhook-util \

src_popnhook1		:= \
    avs-boot.c \
    config-eamuse.c \
    config-gfx.c \
    config-sec.c \
    d3d9.c \
    dllmain.c \
    filesystem.c \
