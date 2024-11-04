LIBS:=ncurses readline

CC:=gcc
CFLAGS += $(if $(SAN),-fsanitize=${SAN})
CPPFLAGS:=-I config/ ${shell pkg-config --cflags ${LIBS}}
LDLIBS=${shell pkg-config --libs ${LIBS}}
LEX:=flex

ifeq (${DEBUG}, 1)
  CFLAGS += -O0 -ggdb -Wall -Wpedantic
else
  CFLAGS += -O3 -flto=auto -fomit-frame-pointer
endif

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

OUTPUT:=csope

main: ${object}
	${LINK.c} ${object} -o ${OUTPUT} ${LDLIBS}

object/%.o: source/%.c
	${COMPILE.c} $< -o $@

source/%.c: source/%.l
	${LEX} -o $@ $<

source/%.c: source/%.y
	${YACC} -o $@ $<

install: ${OUTPUT}
	cp ${OUTPUT} /usr/bin/

clean:
	-${RM} ${GENLEX}
	-${RM} ${GENYACC}
	-${RM} ${object}
	-${RM} ${OUTPUT}
