dlls		+= ddrio-p3io

ldflags_ddrio-p3io:= \
	-lsetupapi \
    -lws2_32 \

libs_ddrio-p3io	:= \
	core \
	extio \
	extiodrv \
	p3io \
	p3iodrv \
	util \
	iface-core \
	security \

src_ddrio-p3io	:= \
	config.c \
	ddrio.c \
