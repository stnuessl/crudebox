#
# Dockerfile
# 
# Used to create an image containing alpine linux
# with all required dependencies to build the project.
#

FROM alpine:latest
RUN apk add --no-cache \
    binutils \
    cairo-dev \
    findutils \
    freetype-dev \
    gcc \
    libc-dev \
    libxcb-dev \
    libxkbcommon-dev \
    make \
    pkgconf \
    wayland-dev \
    wayland-protocols \
    xcb-util-keysyms-dev
RUN adduser --disabled-password --shell /usr/bin/bash alpine
USER alpine
WORKDIR /home/alpine
CMD /bin/sh

