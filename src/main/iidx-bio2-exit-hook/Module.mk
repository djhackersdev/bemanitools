dlls		    += iidx-bio2-exit-hook

ldflags_iidx-bio2-exit-hook      := \
    -lsetupapi \
    -lpsapi \

libs_iidx-bio2-exit-hook       := \
    core \
    bio2drv \
    hook \
    util \
    iface-core \

src_iidx-bio2-exit-hook       := \
    main.c \
