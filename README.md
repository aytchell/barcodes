# Barcode-sender

This is a tiny program with the following task:
 1. Search all attached input devices for a barcode scanner (or RFID reader)
 1. Grab the device so all input is exclusively routed to this program
 1. Send each scanned barcode to a given REST endpoint
 1. Repeat until scanner is unplugged

## Compiling

You need to install the development versions of
 * [libevdev](https://www.freedesktop.org/software/libevdev/doc/latest/index.html)
 * [libcurl](https://curl.se/libcurl/)

If these two are installed, there's a little Makefile so can can call `make`.

The "build system" is intentionally kept simple to be easier portable to
smaller platforms.

## Installation

Copy built executable to `/usr/local/bin/barcodes`
Change owner to root: `chown root.root /usr/local/bin/barcodes`

(the program will strip its root priviledges as soon as the correct input
device is grabbed)

Add this custom udev rule in `/etc/udev/rules.d/90-barcodes.rules`

```
SUBSYSTEM=="usb", ACTION=="add", ATTRS{idVendor}=="28e9", ATTRS{idProduct}=="03d9", RUN+="/usr/local/bin/barcodes"
```

Either reboot or call
```
# udevadm control --reload-rules && udevadm trigger
```


## License

SPDX: GPL-3.0-or-later

Barcode-sender -- a program for sending barcode contents to REST APIs

Copyright (C) 2022 Hannes Lerchl

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along with
this program. If not, see
[https://www.gnu.org/licenses/](https://www.gnu.org/licenses/).

