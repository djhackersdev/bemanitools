libs		+= sdk-hook

ldflags_sdk-hook   := \
    -lshlwapi \

libs_sdk-hook	:= \
	util \

src_sdk-hook	:= \
    dllentry.c \
	hooks-config.c \
	inject-config.c \
	logger-config.c \
