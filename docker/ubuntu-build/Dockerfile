#
# Dockerfile
#
# Used to create an image containing ubuntu linux
# with all required dependencies to build the project.
#

FROM ubuntu:rolling
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get --assume-yes install \
    bsdmainutils \
    clang \
    clang-tools \
    cppcheck \
    curl \
    findutils \
    gcc \
    lcov \
    libcairo2-dev \
    libcmocka-dev \
    libfreetype-dev \
    libwayland-dev \
    libx11-xcb-dev \
    libxcb-keysyms1-dev \
    libxkbcommon-dev \
    make \
    pkg-config \
    universal-ctags \
    uuid-runtime \
    wayland-protocols \
&& ln -sf $(ls /bin/clang-extdef-* | head -n 1) /bin/clang-extdef-mapping \
&& useradd \
    --shell /usr/bin/bash \
    --no-create-home \
    ubuntu
USER ubuntu
CMD [ "/usr/bin/bash" ]

