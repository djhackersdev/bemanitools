libs            += vigemstub
imps            += ViGEmClient

cppflags_vigemstub  := \
    -I src/imports \

libs_vigemstub  := \
 
src_vigemstub   := \
    helper.c \

# This is a dummy module so that the client only gets import libraries generated once
