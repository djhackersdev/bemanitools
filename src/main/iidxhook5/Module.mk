avsdlls         += iidxhook5

ldflags_iidxhook5   := \
    -liphlpapi \

deplibs_iidxhook5   := \
    avs \

avslibs_iidxhook5 := \
    avs-ext \

libs_iidxhook5      := \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
    acioemu \
    hook \
    hooklib \
    cconfig \
    ezusb \
    iface \
    iface-io \
    iface-core \
    module \
    util \
    mxml \

src_iidxhook5       := \
    dllmain.c \
    iidxhook5.c \
    ifs-snd-redir.c \
