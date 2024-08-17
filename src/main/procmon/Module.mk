dlls		+= procmon

libs_procmon	:= \
	core \
	hook \
	util \
	iface-core \

src_procmon	:= \
	bt-hook.c \
	config.c \
	dllmain.c \
	file.c \
	module.c \
	procmon.c \
	thread.c \

