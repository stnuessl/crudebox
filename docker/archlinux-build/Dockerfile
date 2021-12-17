#
# Dockerfile
#
# Used to create an image containing archlinux
# with all required dependencies to build the project.
#

FROM archlinux:base
RUN pacman -Syu --noconfirm \
    cairo \
    clang \
    cppcheck \
    ctags \
    freetype2 \
    gcc \
    libxcb \
    libxkbcommon \
    make \
    perl \
    pkgconfig \
    python \
    util-linux \
    wayland \
    wayland-protocols \
    xcb-util-keysyms \
&& pacman -Scc --noconfirm \
&& useradd \
    --shell /usr/bin/bash \
    --no-create-home \
    arch
USER arch
COPY entrypoint.sh /usr/local/bin/
ENTRYPOINT [ "/usr/bin/bash", "entrypoint.sh" ]
CMD [ "/usr/bin/bash" ]

