# foxdo-gtk - GTK frontend for foxdo
# Copyright (C) 2022 Ján Gajdoš

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

CC ?= g++
CFLAGS ?= -Wall -g -O2 `pkg-config gtkmm-3.0 --cflags --libs` -lstdc++ -L. -lfoxdo -lcrypt -lfoxstd
.DEFAULT_GOAL = all

prepare:
	$(MAKE) -C foxdo all
	cp foxdo/foxstd.h ./
	cp foxdo/libfoxstd.a ./
	cp foxdo/libfoxdo.a ./

all: prepare
	$(CC) main.cpp -o main $(CFLAGS)

rootable:
	chown root:root ./main
	chmod ugo= ./main
	chmod +s ./main
	chmod u+rwx,g=rx,o=rx ./main

install: rootable
	cp -p main /usr/bin/foxdo-gtk

uninstall:
	rm /usr/bin/foxdo-gtk

clean:
	$(MAKE) -C foxdo clean
	rm foxstd.h
	rm libfoxstd.a
	rm libfoxdo.a