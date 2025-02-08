exes            += d3d9-monitor-check \

ldflags_d3d9-monitor-check   := \
    -ld3d9 \
    -ldwmapi \
    -lgdi32 \
    -ld3dx9 \

libs_d3d9-monitor-check     := \
    util \

src_d3d9-monitor-check      := \
    main.c \
