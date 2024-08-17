exes            += p3io-ddr-tool \

ldflags_p3io-ddr-tool   := \
    -lsetupapi \

libs_p3io-ddr-tool     := \
    core \
    extiodrv \
    extio \
    p3iodrv \
    p3io \
    hook \
    util \
    iface-core \

src_p3io-ddr-tool      := \
    main.c \
    mode-test.c \
