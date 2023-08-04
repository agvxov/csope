# CC=gcc
CFLAGS:=-Wall -Wextra -Wpedantic
CPPFLAGS:=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600
LDLIBS=-I ${CHDRD} -lncurses -ltinfo -lreadline
LEX:=flex

LEXD:=src/
LEXF:=$(shell find ${LEXD} -iname '*.l')
GENLEX:=$(subst .l,.c,${LEXF})

SRCD:=src/
OBJD:=obj/
SRC:=$(shell find ${SRCD} -iname '*.c') ${GENLEX}
OBJ:=$(subst .c,.o,$(subst ${SRCD},${OBJD},${SRC}))

HDRD:=${SRCD}
CHDRD:=${OBJD}
HDR:=$(shell find ${HDRD} -iname '*.h')
CHDR:=$(addsuffix .gch,$(subst ${HDRD},${CHDRD},${HDR}))

OUTPUT:=csope

ifeq (${DEBUG},1)
	CFLAGS += -Og -ggdb
else
	CFLAGS += -O3 -flto=auto -fomit-frame-pointer
endif

ifdef SAN
	CFLAGS += -fsanitize=${SAN}
endif

main: ${CHDR} ${OBJ}
	${LINK.c} ${OBJ} -o ${OUTPUT} ${LDLIBS}

obj/%.o: src/%.c
	${COMPILE.c} $< -o $@

src/%.c: src/%.l
	${LEX} -o $@ $<

obj/%.h.gch: src/%.h
	${CC} $< -o $@

clean:
	-rm ${CHDR}
	-rm ${GENLEX}
	-rm ${OBJ}
	-rm ${OUTPUT}
