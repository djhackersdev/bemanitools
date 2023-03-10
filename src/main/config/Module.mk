exes            += config
rc_config       := config.rc
cppflags_config := -DUNICODE

libs_config     := \
    eamio \
    geninput \
    util \

ldflags_config  := \
    -lcomctl32 \
    -lcomdlg32 \
    -lgdi32 \
    -mwindows \

src_config      := \
    analogs.c \
    bind-adv.c \
    bind.c \
    bind-light.c \
    buttons.c \
    eam.c \
    gametype.c \
    lights.c \
    main.c \
    schema.c \
    snap.c \
    spinner.c \
    usages.c \

