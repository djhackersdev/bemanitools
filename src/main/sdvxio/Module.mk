dlls		+= sdvxio

ldflags_sdvxio      := \
    -lwinmm

libs_sdvxio	:= \
    iface \
    module \
    iface-io \
    iface-core \
    util \

src_sdvxio	:= \
	sdvxio.c \

