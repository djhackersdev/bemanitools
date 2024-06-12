avsdlls			+= sdvxhook

deplibs_sdvxhook	:= \
	avs \

avslibs_sdvxhook := \
    avs-ext \

libs_sdvxhook		:= \
	core \
	acioemu \
	hook \
	hooklib \
	util \
    iface \
    iface-io \
    iface-core \
    module \

src_sdvxhook		:= \
	acio.c \
	dllmain.c \
	gfx.c \
	kfca.c \
	lcd.c \

