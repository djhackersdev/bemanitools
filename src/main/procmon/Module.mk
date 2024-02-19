dlls		+= procmon

libs_procmon	:= \
	core \
	hook \
	util \

src_procmon	:= \
	file.c \
	module.c \
	procmon.c \
	thread.c \

