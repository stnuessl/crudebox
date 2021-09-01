#
# Dockerfile
# Creates a docker image suitable to build crudebox
#

FROM archlinux:base
RUN pacman -Syu --noconfirm \
cairo \
clang \
freetype2 \
gdb \
git \
libxcb \
libxkbcommon \
make \
neovim \
pkgconfig \
valgrind \
wayland \
wayland-protocols \
weston \
xcb-util-keysyms
RUN useradd --create-home --uid 1000 --shell /usr/bin/bash docker
USER docker
CMD /usr/bin/bash
