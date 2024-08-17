dlls		    += iidx-ezusb-exit-hook

ldflags_iidx-ezusb-exit-hook      := \
    -lpsapi \

libs_iidx-ezusb-exit-hook       := \
    core \
    ezusb-iidx \
    hook \
    util \
    iface-core \

src_iidx-ezusb-exit-hook       := \
    main.c \
