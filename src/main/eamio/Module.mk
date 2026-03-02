dlls            += eamio
libs_eamio      := geninput util
ldflags_eamio   := -lwinscard
src_eamio       := \
    eam-api.c \
    eam-impl.c \
    eam-s11n.c \
    eam-scard.c
