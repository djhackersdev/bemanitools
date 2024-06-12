dlls		+= ddrio-smx
imps		+= SMX

deplibs_ddrio-smx := \
	SMX \

libs_ddrio-smx	:= \
	core \
	geninput \
	util \
	iface-core \
	iface \
	module \

src_ddrio-smx	:= \
	ddrio.c \

