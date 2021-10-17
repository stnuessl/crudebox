
# Dockerfile
#
# Used to create an image containing Fedora
# with all required dependencies to build the project.
#

FROM fedora:latest
RUN dnf install --assumeyes \
    cairo-devel \
    clang \
    clang-tools-extra \
    ctags \
    freetype-devel \
    gcc \
    libxcb-devel \
    libxkbcommon-devel \
    make \
    perl \
    pkgconfig \
    python \
    util-linux \
    wayland-devel \
    wayland-protocols-devel \
    xcb-util-keysyms-devel \
&& dnf clean dbcache --assumeyes \
&& useradd \
    --shell /usr/bin/bash \
    --no-create-home \
    fedora
USER fedora
CMD [ "/usr/bin/bash" ]

