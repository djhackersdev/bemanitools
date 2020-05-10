FROM fedora:31

LABEL description="Build environment for bemanitools"

RUN yum -y install \
    git \
    make \
    zip \
    clang \
    mingw64-gcc.x86_64 \
    mingw32-gcc.x86_64 \
    wine.x86_64

RUN mkdir /bemanitools
WORKDIR /bemanitools

# Order optimized for docker layer caching
COPY run-tests-wine.sh run-tests-wine.sh
COPY CHANGELOG.md CHANGELOG.md
COPY CONTRIBUTING.md CONTRIBUTING.md
COPY LICENSE LICENSE
COPY GNUmakefile GNUmakefile
COPY Module.mk Module.mk
COPY README.md README.md
COPY doc doc
COPY dist dist
COPY src src
# .git folder required or building fails when version is generated
COPY .git .git

# Building
RUN make