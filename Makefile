# See LICENSE file for copyright and license details.
# nfy - a minimal and daemonless notification program for X
.POSIX:

include config.mk

BIN = nfy
DIST = ${BIN}-${VERSION}
MAN1 = ${BIN}.1

SRC = nfy.c
OBJ = ${SRC:.c=.o}

all: options ${BIN}

options:
	@echo ${BIN} build options:
	@echo "CFLAGS	= ${CFLAGS}"
	@echo "LDFLAGS	= ${LDFLAGS}"
	@echo "CC	= ${CC}"

${BIN}: ${OBJ}
	${CC} ${LDFLAGS} ${OBJ} -o $@

.c.o:
	${CC} -c ${CFLAGS} $<

dist: clean
	${MKDIR} ${DIST}
	${CP} -R nfy.c config.h Makefile config.mk ${DIST}
	${TAR} ${DIST}.tar ${DIST}
	${GZIP} ${DIST}.tar
	${RM_DIR} ${DIST}

run:
	./${BIN}

install: all
	${MKDIR} ${DESTDIR}${BIN_DIR} ${DESTDIR}${MAN_DIR}
	${CP} ${BIN} ${BIN_DIR}
	${CP} ${MAN1} ${DESTDIR}${MAN_DIR}
	sed "s/VERSION/${VERSION}/g" < ${MAN1} > ${DESTDIR}${MAN_DIR}/${MAN1}
	chmod 755 ${DESTDIR}${BIN_DIR}/${BIN}
	chmod 644 ${DESTDIR}${MAN_DIR}/${MAN1}

uninstall:
	${RM} ${DESTDIR}${BIN_DIR}/${BIN}
	${RM} ${DESTDIR}${MAN_DIR}/${MAN1}

clean:
	${RM} ${BIN} ${OBJ} ${DIST}.tar.gz

.PHONY: all options clean dist install uninstall run
