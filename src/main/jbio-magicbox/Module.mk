dlls        += jbio-magicbox
imps		+= ch341

deplibs_jbio-magicbox := \
	ch341 \
    iface \
    module \
    iface-core \
    util \

src_jbio-magicbox          := \
    jbio.c \

libs_jbio-magicbox      := \
    util \
