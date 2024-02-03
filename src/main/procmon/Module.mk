dlls		+= procmon

libs_procmon	:= \
	hook \
	util \

src_procmon	:= \
	file.c \
	module.c \
	procmon.c \
	thread.c \

