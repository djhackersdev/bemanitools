dlls		+= ddrio-mm-aux

ldflags_ddrio-mm-aux:= \
	-lhid \
	-lsetupapi \

libs_ddrio-mm-aux	:= \
	geninput \
	mm \
	util \

src_ddrio-mm-aux	:= \
	ddrio.c \

