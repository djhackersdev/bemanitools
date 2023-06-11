dlls		+= ddrio-p3io

ldflags_ddrio-p3io:= \
	-lsetupapi \

libs_ddrio-p3io	:= \
	cconfig \
	extio \
	extiodrv \
	p3io \
	p3iodrv \
	util \

src_ddrio-p3io	:= \
	config.c \
	ddrio.c \
