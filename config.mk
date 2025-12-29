# See LICENSE file for copyright and license details.
# nfy version
VERSION = 0.3

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
# OpenBSD
#MANPREFIX = ${PREFIX}/man

# includes and libs
# FreeBSD
X11INC = /usr/local/include
X11LIB = /usr/local/lib

# Linux/OpenBSD
#X11INC = /usr/X11R6/include
#X11LIB = /usr/X11R6/lib

FREETYPELIBS = -lfontconfig -lXft
FREETYPEINC = ${X11INC}/freetype2
# Linux
#FREETYPEINC = /usr/include/freetype2

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L \
	   -D_XOPEN_SOURCE=700 -DVERSION=\"${VERSION}\"
CFLAGS = -std=c99 -pedantic -Wall -Os -I${X11INC} -I${FREETYPEINC} ${CPPFLAGS}
LDFLAGS = -L${X11LIB} ${FREETYPELIBS} -lX11 -lXrandr

# compiler
CC = cc
