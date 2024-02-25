dlls                += geninput

ldflags_geninput    := \
    -lhid \
    -lsetupapi \

libs_geninput       := \
    core \
    util \

src_geninput        := \
    dev-list.c \
    guid.c \
    hid.c \
    hid-generic.c \
    hid-generic-strings.c \
    hid-meta-in.c \
    hid-meta-out.c \
    hid-mgr.c \
    hid-report-in.c \
    hid-report-out.c \
    hotplug.c \
    input.c \
    io-thread.c \
    kbd.c \
    kbd-data.c \
    mapper.c \
    mapper-s11n.c \
    mouse.c \
    msg-thread.c \
    pacdrive.c \
    ri.c \

