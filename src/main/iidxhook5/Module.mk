avsdlls         += iidxhook5

ldflags_iidxhook5   := \
    -liphlpapi \

deplibs_iidxhook5   := \
    avs \

libs_iidxhook5      := \
	avs-util \
    core \
    iidxhook-util \
    ezusb-emu \
    ezusb-iidx-16seg-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
    eamio \
    acioemu \
    hook \
    hooklib \
    iidxio \
    cconfig \
    util \
    ezusb \

src_iidxhook5       := \
    dllmain.c \
    ifs-snd-redir.c \
