avsdlls         += pnmhook2

ldflags_pnmhook2   := \
    -liphlpapi \

deplibs_pnmhook2   := \
    avs \

libs_pnmhook2      := \
    iidxhook-util \
    cconfig \
    eamio \
    acioemu \
    hook \
    hooklib \
    util \
    ezusb \

src_pnmhook2      := \
    acio.c \
    dllmain.c \
