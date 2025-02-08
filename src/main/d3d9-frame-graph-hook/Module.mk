dlls		    += d3d9-frame-graph-hook

ldflags_d3d9-frame-graph-hook   := \
    -ld3d9 \
    -ldwmapi\
    -lgdi32 \

libs_d3d9-frame-graph-hook       := \
    hook \
    imgui-bt \
    imgui-debug \
    imgui \
    util \

src_d3d9-frame-graph-hook       := \
    main.c \
