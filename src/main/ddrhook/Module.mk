avsdlls			+= ddrhook

deplibs_ddrhook	:= \
	avs \

libs_ddrhook		:= \
	acioemu \
	p3ioemu \
	p3io \
	hook \
	hooklib \
	security \
	util \
	eamio \
	ddrio \

src_ddrhook		:= \
	extio.c \
	dllmain.c \
	_com4.c \
	dinput.c \
	gfx.c \
	guid.c \
	master.c \
	misc.c \
	monitor.c \
	p3io.c \
	usbmem.c \
	spike.c

