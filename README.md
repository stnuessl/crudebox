# crudebox (DRAFT)

A simple and fast program launcher.

## Overview
* [crudebox (DRAFT)](README.md#crudebox-(draft))
    * [Overview](README.md#overview)
    * [About](README.md#about)
        * [Features](README.md#features)
        * [Missing Features](README.md#missing-features)
    * [Installation](README.md#installation)
        * [Prerequisites](README.md#prerequisites)
        * [Dependencies](README.md#dependencies)
            * [General](README.md#general)
            * [Arch Linux](README.md#arch-linux)
        * [Building](README.md#building)
        * [Installation](README.md#installation)
        * [Deinstallation](README.md#deinstallation)
    * [Usage](README.md#usage)
        * [Keyboard Shortcuts](README.md#keyboard-shortcuts)
        * [Configuration File](README.md#configuration-file)
        * [Environment](README.md#environment)
        * [Read from standard input](README.md#read-from-standard-input)
        * [Cache](README.md#cache)

## About

> Perfection is achieved, not when there is nothing more to add, 
> but when there is nothing left to take away.

__Antoine de Saint-Exupery__

### Features

* Can be built for x11 or wayland
* Widget style customization

### Missing Features

* No utf-8 support
* No scrolling

## Installation

### Prerequisites

* git
* gcc / clang
* pkg-config
* make

### Dependencies

#### General

When buildung __crudebox__ the following libraries must be available in
pkg-config:

* [cairo (1.17.4)](https://www.cairographics.org/)
* [freetype2 (2.10.4)](https://www.freetype.org/index.html)
* [xkbcommon (1.1.0)](https://xkbcommon.org/)

If __crudebox__ shall be built for __X11__, pkg-config will also need the
* [xcb (1.14)](https://xcb.freedesktop.org/)
* [xcb-keysyms (0.4.0)](https://xcb.freedesktop.org/XcbUtil/)

libraries. If instead __crudebox__ shall be built for __wayland__, pkg-config 
will need the
* [wayland-client (1.19.0)](https://wayland.freedesktop.org/)

library. Additionally 
* [wayland-protocols (1.20)](https://wayland.freedesktop.org/)

needs to be installed to generate the xdg-shell client implementation.

#### Arch Linux

If you wish to use __crudebox__ under __X11__, run the command below
to install all needed dependencies.
```
# pacman -Syu cairo freetype2 libxcb libxkbcommon xcb-util-keysyms
```


If you wish to use __crudebox__ under __Wayland__, run the command below
to install all needed dependencies.
```
# pacman -Syu cairo freetype2 wayland wayland-protocols libxkbcommon
```

### Building

```
$ git clone https://github.com/stnuessl/crudebox
$ cd crudebox
$ make
```

### Installation

Install to _/usr/local/bin_ 
```
# make install
```

### Deinstallation

Uninstall __crudebox__ by navigating into the project directory and run
```
# make uninstall
```
or just simply
```
# rm /usr/local/bin/crudebox
```

The cache and configuration files need to be removed manually.

## Usage

### Keyboard Shortcuts

| Key           | Alternative           | Description               |
|---------------|-----------------------|----------------------------
| Escape        | Ctrl + c              | Quit __crudebox__         |
| Enter         |                       | Execute selected item     |
| Shift + Tab   | Arrow Up              | Select previous item      |
| Tab           | Arrow Down            | Select next item          |
| Ctrl + w      |                       | Clear __crudebox__ input  |
| Page Up       | Home                  | Select top item           |
| Page Down     | End                   | Select bottom item        |

### Configuration File

__crudebox__ will not work without a configuration file in one of the following
paths:

* ~/.config/crudebox/config
* ~/.config/crudebox/crudebox.conf
* ~/.crudebox.conf
* /etc/crudebox/config
* /etc/crudebox/crudebox.conf
* /etc/crudebox.conf

Here is an example configuration file that can be used:
```
#
# crudebox config file
#

[widget]

frame = 0x4c7899
line-width = 2

[font]
path = /usr/share/fonts/TTF/Hack-Regular.ttf
size = 16

[line-edit]
fg = 0xcccccc
bg = 0x282828

[list-view]
size = 15

fg = 0xcccccc
bg1 = 0x282828
bg2 = 0x222222
fg-sel = 0x285577
bg1-sel = 0x181818
bg2-sel = 0x181818
lines = 0x282828 
```

Please note that it's likely that you do not have the font "Hack" installed and
__crudebox__ will probably not work at all. To fix this, replace the font path 
in the configuration with a path that is existing on your system.

### Environment

```
$ PATH=/usr/local/bin crudebox
```
```
$ PATH="${PATH}:~/bin" crudebox
```

### Read from standard input

__crudebox__ automatically detects if data is available on standard input.


$ ls --color=never ~/ | crudebox

### Cache

Improve __crudebox__ initialization performance by enabling the cache:
```
$ mkdir -p ~/.cache/crudebox
```

Disable the __crudebox__ cache:
```
$ rm -rf ~/.cache/crudebox
```
