avsdlls		    += pnmhook1

ldflags_pnmhook1   := \
    -lws2_32 \
    -liphlpapi \
    
libs_pnmhook1     := \
    iidxhook-util \
    ezusb-emu \
    ezusb2-emu \
    ezusb2-iidx-emu \
    ezusb-iidx-emu \
    security \
    eamio \
    iidxio \
    hook \
    hooklib \
    util \
    ezusb \

src_pnmhook1       := \
    d3d9.c \
    dllmain.c \
