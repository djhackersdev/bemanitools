dlls		+= ddrio-mm

ldflags_ddrio-mm:= \
	-lhid \
	-lsetupapi \

libs_ddrio-mm	:= \
	core \
	mm \
	util \

src_ddrio-mm	:= \
	ddrio.c \

