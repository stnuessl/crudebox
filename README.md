# crudebox (DRAFT)

A simple and fast program launcher.

## Overview
* [crudebox (DRAFT)](README.md#crudebox-(draft))
    * [About](README.md#about)
        * [Available Features](README.md#available-features)
        * [Missing Features](README.md#missing-features)
    * [Installation](README.md#installation)
        * [Prerequisites](README.md#prerequisites)
        * [Dependencies](README.md#dependencies)
        * [Building](README.md#building)
        * [Installation](README.md#installation)
        * [Deinstallation](README.md#deinstallation)
    * [Usage](README.md#usage)
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

## Installation

### Prerequisites

* git
* gcc / clang
* pkg-config
* make

### Dependencies

When buildung __crudebox__ the following libraries must be available in
pkg-config:

* cairo 
* freetype2
* xkbcommon

If __crudebox__ shall be built for __X11__, pkg-config will also need the
* xcb
* xcb-keysyms

libraries. If instead __crudebox__ shall be built for __wayland__, pkg-config 
will need the
* wayland-client

library.

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
