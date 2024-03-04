dlls		+= procmon

libs_procmon	:= \
	core \
	hook \
	util \

src_procmon	:= \
	dllmain.c \
	file.c \
	module.c \
	procmon.c \
	thread.c \

