CC = gcc
CFLAGS = --std=c11 -Werror -Wall -Wpedantic -Wstrict-prototypes \
-Wmissing-prototypes -Wmissing-declarations -march=native -Os \
-fstack-check -fstack-protector-strong -Wformat-nonliteral \
-Wformat-security
LIBS = -lncurses

OUTPUT = todoview
SOURCES != find . -name '*.c'
OBJECTS = ${SOURCES:.c=.o}

${OUTPUT}: ${OBJECTS}
	${CC} -o $@ $^ ${LIBS}

include ${SOURCES:.c=.deps}

%.deps: %.c
	@set -e; rm -f $@; \
	${CC} -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o: %.c
	${CC} -o $@ -c ${CFLAGS} $<

all: ${OUTPUT}

clean:
	@rm -f ${OUTPUT}
	@rm -f *.deps
	@rm -f *.deps.*
	@rm -f *.o

.PHONY: clean
