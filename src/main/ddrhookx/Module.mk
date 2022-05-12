avsdlls			+= ddrhookx

deplibs_ddrhookx	:= \
	avs \

libs_ddrhookx		:= \
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

src_ddrhookx		:= \
	avs-boot.c \
	dllmain.c \
	config-ddrhookx.c \
	config-eamuse.c \
	config-gfx.c \
	config-security.c \
	d3d9.c \
	master.c \
	filesystem.c \
