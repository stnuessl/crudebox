[![CI-Build](https://github.com/stnuessl/crudebox/actions/workflows/main.yaml/badge.svg)](https://github.com/stnuessl/crudebox/actions/workflows/main.yaml)

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
            * [CRUDEBOX_CACHE](README.md#crudebox_cache)
            * [CRUDEBOX_CONFIG](README.md#crudebox_config)
            * [PATH](README.md#path)
            * [XDG_CACHE_HOME](README.md#xdg_cache_home)
            * [XDG_CONFIG_HOME](README.md#xdg_config_home)
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

* ${XDG_CONFIG_HOME}/crudebox/config
* ${XDG_CONFIG_HOME}/crudebox/crudebox.conf
* ${HOME}/.crudebox
* ${HOME}/.crudebox.conf
* /etc/crudebox/config
* /etc/crudebox/crudebox.conf
* /etc/crudebox.conf

Alternatively, see [CRUDEBOX_CONFIG](README.md#crudebox_config) on how to 
specify a different path for the __crudebox__ configuration file.

Here is an example configuration file that can be used:
```
#
# crudebox configuration file
#
# The color scheme defined in this configuration file is targeted to blend in
# well with the default i3-wm's color scheme and vim's gruvbox-dark scheme.
#

[widget]

# Set the widget's frame color
frame = 0x4c7899
# Set the widget's frame line width
line-width = 2

[font]

# Set the font used by crudebox
path = /usr/share/fonts/TTF/Hack-Regular.ttf
# Set the font size.
size = 16

[line-edit]

# Foreground color
fg = 0xcccccc
# Background color
bg = 0x282828

[list-view]

# Set the number of entries shown in the menu
size = 15
# Foreground color
fg = 0xcccccc
# Background colors
bg1 = 0x282828
bg2 = 0x222222
# Foreground and background colors for the selected item
fg-sel = 0x285577
bg1-sel = 0x181818
bg2-sel = 0x181818
# Separation line color
lines = 0x282828
```

Please note that it's likely that you do not have the font "Hack" installed and
__crudebox__ will probably not work at all. To fix this, replace the font path 
in the configuration with a path that is existing on your system.

### Environment

There are several enviroment variables which can be used to influence the
behavior of __crudebox__.

#### CRUDEBOX_CACHE

The _CRUDEBOX_CACHE_ variable can be used to directly specify the cache file
used by __crudebox__.

```
$ CRUDEBOX_CACHE=/tmp/crudebox.cache crudebox
```

#### CRUDEBOX_CONFIG

The _CRUDEBOX_CONFIG_ variable can be used to directly specify the configuration
file used by __crudebox__.

```
$ CRUDEBOX_CONFIG=~/.crudebox.ini crudebox
```

#### PATH

The _PATH_ environment variable specifies a colon separated list of 
directories which __crudebox__ uses to search for executable programs.

```
$ PATH=/usr/local/bin crudebox
```
```
$ PATH="${PATH}:~/bin" crudebox
```

#### XDG_CACHE_HOME

The cache directory used by __crudebox__ can be changed via the
_XDG_CACHE_HOME_ environment variable. E.g. running __crudebox__ with 

```
$ XDG_CACHE_HOME=/run/user/${UID}/cache crudebox
```

will make the program use _/run/user/${UID}/cache/crudebox/cache_ as the 
cache file.

#### XDG_CONFIG_HOME

```
$ XDG_CONFIG_HOME=~/configuration crudebox
```

### Read from standard input

__crudebox__ automatically detects if data is available on standard input.

```
$ ls --color=never /usr/local/bin | crudebox
```

### Cache

__crudebox__ uses by default the cache directory _${HOME}/.cache/crudebox_ for
its cache file. If it detects that this directory exists it will automatically
use it for its cache file. 
```
$ mkdir -p ~/.cache/crudebox
```

Deleting the same directory will disable __crudebox__ cache:
```
$ rm -rf ~/.cache/crudebox
```

The path of the cache can be changed via environment variables, see
[CRUDEBOX_CACHE](README.md#crudebox_cache).

