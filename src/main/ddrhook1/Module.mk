avsdlls			+= ddrhook1

ldflags_ddrhook1   := \
    -lws2_32 \

deplibs_ddrhook1	:= \
	avs \

libs_ddrhook1		:= \
	acioemu \
	cconfig \
	ddrhook-util \
	p3ioemu \
	p3io \
	hook \
	hooklib \
	util \
	eamio \
	ddrio \
	security \

src_ddrhook1		:= \
	avs-boot.c \
	dllmain.c \
	config-ddrhook1.c \
	config-eamuse.c \
	config-gfx.c \
	config-security.c \
	master.c \
	filesystem.c \
