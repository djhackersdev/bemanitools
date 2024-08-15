avsdlls			+= ddrhook2

deplibs_ddrhook2	:= \
	avs \

avslibs_ddrhook2 := \
    avs-ext \

libs_ddrhook2		:= \
	core \
	acioemu \
	ddrhook-util \
	p3ioemu \
	p3io \
	hook \
	hooklib \
	util \
	security \
	iface-core \
	iface-io \
	module \

src_ddrhook2		:= \
	dllmain.c \
	master.c \

