avsdlls			+= ddrhook

deplibs_ddrhook	:= \
	avs \

libs_ddrhook		:= \
	acioemu \
	ddrhook-util \
	p3ioemu \
	p3io \
	hook \
	hooklib \
	util \
	eamio \
	ddrio \

src_ddrhook		:= \
	extio.c \
	dllmain.c \
	_com4.c \
	master.c \
	p3io.c \
	usbmem.c \
	spike.c

