DEBUG:=1
GCC:=0

CC:=gcc
CFLAGS:=-Wall -Wextra -Wpedantic
CFLAGS +=$(if $(DEBUG),-O0 -ggdb,-O3 -flto=auto -fomit-frame-pointer)
CFLAGS +=$(if $(SAN),-fsanitize=${SAN})
CPPFLAGS:=-I config/ -I ${CHDRD} ${shell pkg-config --cflags ncurses readline}
LDLIBS=${shell pkg-config --libs ncurses readline}
LEX:=flex

LEXD:=src/
LEXF:=$(shell find ${LEXD} -iname '*.l')
GENLEX:=$(subst .l,.c,${LEXF})

SRCD:=src/
OBJD:=obj/
SRC:=$(shell find ${SRCD} -iname '*.c') ${GENLEX}
OBJ:=$(subst .c,.o,$(subst ${SRCD},${OBJD},${SRC}))

HDRD:=${SRCD}
CONFD:=config/
CHDRD:=${OBJD}
HDR:=$(shell find ${HDRD} ${CONFD} -iname '*.h')
CHDR:=$(addsuffix .gch,$(subst ${HDRD},${CHDRD},$(subst ${CONFD}, ${CHDRD}, ${HDR})))

OUTPUT:=csope

main: ${CHDR} ${OBJ}
	${LINK.c} ${OBJ} -o ${OUTPUT} ${LDLIBS}

obj/%.o: src/%.c
	${COMPILE.c} $< -o $@

src/%.c: src/%.l
	${LEX} -o $@ $<

obj/%.h.gch: src/%.h
	${CC} $< -o $@

obj/%.h.gch: config/%.h
	${CC} $< -o $@

clean:
	-rm ${CHDR}
	-rm ${GENLEX}
	-rm ${OBJ}
	-rm ${OUTPUT}
