dlls		+= procmon

libs_procmon	:= \
	core \
	hook \
	util \

src_procmon	:= \
	config.c \
	dllmain.c \
	file.c \
	hook.c \
	module.c \
	procmon.c \
	thread.c \

