# Barcode-sender

This is a tiny program with the following task:
 1. Search all attached input devices for a barcode scanner (or RFID reader)
 1. Grab the device so all input is exclusively routed to this program
 1. Send each scanned barcode to a given REST endpoint
 1. Exit when scanner is unplugged

## Compiling

You need to install the development versions of
 * [libevdev](https://www.freedesktop.org/software/libevdev/doc/latest/index.html)
    -- required for reading input devices like keyboards or barcode scanners
 * [libcurl](https://curl.se/libcurl/) -- required for sending data to a REST API

If these two are installed, there's a small Makefile so one can call `make`.

The "build system" is intentionally kept simple to be easier portable to
smaller platforms.

## Installation

### Binary

Installation has to be done manually. So it's mostly up to you where you
store the binary. A reasonable location would be `/usr/local/bin/barcodes`

Change owner to root: `chown root.root /usr/local/bin/barcodes`

(the program will strip its root priviledges as soon as the correct input
device is grabbed)

### Configuration

In the `src/` directory there a configuration file called `barcodes.conf`.
Copy this to `/etc/`; that's where the application will search for it.

Have a look into the config file and change it according to your needs.
The file contains some documentation which will guide you.

### Automatic startup

I experimented with `udev` to start the program as soon as the correct device
is plugged in. Turns out that `udev` is able to `RUN` a program but this is
done in a sandbox and the program will get killed after a short while.

A better attempt would probably be to use the `systemd.device` mechanism but
I didn't yet find time to figure this out (please add a PR if this works for
you).

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

