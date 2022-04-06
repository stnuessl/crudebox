#
# Dockerfile
#
# Used to create an image containing ubuntu linux
# with all required dependencies to build the project.
#

FROM ubuntu:21.10
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get --assume-yes install \
    bsdmainutils \
    clang \
    clang-tools \
    curl \
    cppcheck \
    findutils \
    universal-ctags \
    gcc \
    libcairo2-dev \
    libfreetype-dev \
    libwayland-dev \
    libx11-xcb-dev \
    libxcb-keysyms1-dev \
    libxkbcommon-dev \
    make \
    pkg-config \
    uuid-runtime \
    wayland-protocols \
&& ln -sf $(ls /bin/clang-extdef-* | head -n 1) /bin/clang-extdef-mapping \
&& useradd \
    --shell /usr/bin/bash \
    --no-create-home \
    ubuntu
USER ubuntu
CMD [ "/usr/bin/bash" ]

