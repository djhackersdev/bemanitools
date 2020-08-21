dlls		    += ezusb-proxy

ldflags_ezusb-proxy   := \
    -lws2_32 \
    -liphlpapi \
    -lsetupapi \
    -ldbghelp \

libs_ezusb-proxy      := \
    iidxhook-util \
    ezusb-emu \
    ezusb2-emu \
    hooklib \
    hook \
    util \

src_ezusb-proxy       := \
    dllmain.c \
