# dataviz - a simple data visualisation tool

INCS     =
LIBS     = -lm
CPPFLAGS =
CFLAGS   = -g -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS  = -g -s ${LIBS}

SRC = parser.c dataviz.c
OBJ = ${SRC:.c=.o}

all: dataviz

${OBJ}: parser.h dataviz.h

dataviz: ${OBJ}
	@echo cc -o $@
	@cc -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f dataviz ${OBJ}
