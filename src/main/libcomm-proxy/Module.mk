dlls		    += libcomm-proxy

ldflags_libcomm-proxy   := \
    -lws2_32 \
    -liphlpapi \

libs_libcomm-proxy      := \

src_libcomm-proxy       := \
    dllmain.c \
