FROM --platform=amd64 fedora:31@sha256:cbe53d28f54c0f0b1d79a1817089235680b104c23619772473f449f20edd37dd

LABEL description="Build environment for bemanitools"

RUN yum -y install \
    git \
    make \
    zip \
    clang \
    mingw64-gcc.x86_64 \
    mingw32-gcc.x86_64

RUN mkdir /bemanitools
WORKDIR /bemanitools

ENTRYPOINT [ \
    "/bin/bash", \
    "-c" , \
    "cd /bemanitools && \
    make" ]