FROM --platform=amd64 debian:11.6-slim@sha256:f7d141c1ec6af549958a7a2543365a7829c2cdc4476308ec2e182f8a7c59b519

LABEL description="Build environment for bemanitools"

# mingw-w64-gcc has 32-bit and 64-bit toolchains
RUN apt-get update && apt-get install -y --no-install-recommends \
    mingw-w64 \
    mingw-w64-common \
    make \
    zip \
    git \
 && rm -rf /var/lib/apt/lists/*

RUN mkdir /bemanitools
WORKDIR /bemanitools

ENTRYPOINT [ \
    "/bin/sh", \
    "-c" , \
    "cd /bemanitools && \
    make" ]