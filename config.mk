# See LICENSE file for copyright and license details.
# nfy version
VERSION = 0

# paths
PREFIX = /usr/local
MAN_DIR = ${PREFIX}/share/man/man1
BIN_DIR = ${PREFIX}/bin

# includes and libs
#X11INC = /usr/local/include
#X11LIB = /usr/local/lib

# Linux
X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

FREETYPELIBS = -lfontconfig -lXft
#FREETYPEINC = ${X11INC}/freetype2
# Linux
FREETYPEINC = /usr/include/freetype2

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L \
	   -D_XOPEN_SOURCE=700 -DVERSION=\"${VERSION}\"
CFLAGS = -std=c99 -pedantic -Wall -Os -I${X11INC} -I${FREETYPEINC} ${CPPFLAGS}
LDFLAGS = -L${X11LIB} ${FREETYPELIBS} -lX11

# utils
CP = cp -f
RM = rm -f
RM_DIR = rm -rf
MV = mv
MKDIR = mkdir -p
RM_DIR = rm -rf
TAR = tar -cf
GZIP = gzip

# compiler
CC = cc
