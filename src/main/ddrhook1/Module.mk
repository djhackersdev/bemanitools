avsdlls			+= ddrhook1

ldflags_ddrhook1   := \
    -lws2_32 \
    -lshlwapi \

deplibs_ddrhook1	:= \
	avs \

avslibs_ddrhook1 := \
    avs-ext \

libs_ddrhook1		:= \
	sdk-hook \
	core \
	acioemu \
	ddrhook-util \
	p3ioemu \
	p3io \
	hook \
	hooklib \
	util \
	security \
    iface \
	iface-core \
	iface-io \
	module \
	mxml \

src_ddrhook1		:= \
	avs-boot.c \
	ddrhook1.c \
	dllmain.c \
	config-ddrhook1.c \
	config-eamuse.c \
	config-gfx.c \
	config-security.c \
	filesystem.c \
	master.c \
