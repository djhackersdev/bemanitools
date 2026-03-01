dlls		+= ddrio-p4io

deplibs_ddrio-p4io := \

ldflags_ddrio-p4io  := \
    -lsetupapi \

libs_ddrio-p4io	:= \
	p4iodrv \
	aciodrv \
	cconfig \
	util \

src_ddrio-p4io	:= \
	ddrio.c \
	config.c \

