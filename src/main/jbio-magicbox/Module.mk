dlls        += jbio-magicbox
imps		+= ch341

deplibs_jbio-magicbox := \
	ch341 \

src_jbio-magicbox          := \
    jbio.c \

libs_jbio-magicbox      := \
    util \
