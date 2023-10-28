DEBUG:=0

LIBS:=ncurses readline history

CC:=gcc
CFLAGS:=-Wall -Wextra -Wpedantic
CFLAGS +=$(if $(DEBUG),-O0 -ggdb -Wall -Wpedantic,-O3 -flto=auto -fomit-frame-pointer)
CFLAGS +=$(if $(SAN),-fsanitize=${SAN})
CPPFLAGS:=-I config/ -I ${CHDRD} ${shell pkg-config --cflags ${LIBS}}
LDLIBS=${shell pkg-config --libs ${LIBS}}
LEX:=flex

LEXD:=source/
LEXF:=$(shell find ${LEXD} -iname '*.l')
GENLEX:=$(subst .l,.c,${LEXF})

YACCD:=source/
YACCF:=$(shell find ${YACCD} -iname '*.y')
GENYACC:=$(subst .y,.c,${YACCF})

SRCD:=source/
OBJD:=object/
source:=$(shell find ${SRCD} -iname '*.c') ${GENLEX} ${GENYACC}
object:=$(subst .c,.o,$(subst ${SRCD},${OBJD},${source}))

HDRD:=${SRCD}
CONFD:=config/
CHDRD:=${OBJD}
HDR:=$(shell find ${HDRD} ${CONFD} -iname '*.h')
CHDR:=$(addsuffix .gch,$(subst ${HDRD},${CHDRD},$(subst ${CONFD}, ${CHDRD}, ${HDR})))

OUTPUT:=csope

main: ${CHDR} ${object}
	${LINK.c} ${object} -o ${OUTPUT} ${LDLIBS}

object/%.o: source/%.c
	${COMPILE.c} $< -o $@

source/%.c: source/%.l
	${LEX} -o $@ $<

source/%.c: source/%.y
	${YACC} -o $@ $<

object/%.h.gch: source/%.h
	${CC} $< -o $@

object/%.h.gch: config/%.h
	${CC} $< -o $@

install: ${OUTPUT}
	cp ${OUTPUT} /usr/bin/

clean:
	-${RM} ${CHDR}
	-${RM} ${GENLEX}
	-${RM} ${GENYACC}
	-${RM} ${object}
	-${RM} ${OUTPUT}
