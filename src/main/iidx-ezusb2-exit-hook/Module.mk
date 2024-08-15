dlls		    += iidx-ezusb2-exit-hook

ldflags_iidx-ezusb2-exit-hook      := \
    -lpsapi \

libs_iidx-ezusb2-exit-hook       := \
    core \
    hook \
    util \
    iface-core \

src_iidx-ezusb2-exit-hook       := \
    main.c \
