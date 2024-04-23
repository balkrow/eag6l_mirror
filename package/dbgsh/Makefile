CC = ${CROSS_COMPILE}gcc

CFLAGS=-Wall -g -I. -I../ -I../include 
LIBS=-lreadline -lcrypt -ltermcap
STRIP=strip


all:prepare dbgsh
prepare:
	@echo Parse the cmd directory ...
	@sh parse.sh
OBJECT=${patsubst %.c, %.o, ${wildcard *.c cmd/*.c}}
dbgsh:${OBJECT}
	${CC} -o $@ $^ ${LIBS}

install:dbgsh
	${STRIP} dbgsh

.c.o:
.c.h:
clean:
	rm -f *.o cmd/*.o dbgsh
