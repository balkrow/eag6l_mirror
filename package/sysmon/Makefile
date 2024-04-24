# Compiler flags
CFLAGS = -Wall -Wstrict-prototypes -pipe -O2 -fno-strength-reduce -g
LFLAGS =
INCLUDES= -I. -I../include  


OBJECT = sysmon.c thread.c memory.c pqueue.c sigevent.c memtypes.c
PROG_OBJ = devmem.c pid_output.c

all: sysmon

sysmon:  ${OBJECT} ${PROG_OBJ} 
	${CC}  $(CFLAGS) $(DFLAGS) $(INCLUDES) -lreadline -ltermcap -lrt -lcrypt -o $@ $^
	
clean:
	rm -f sysmon rm *.o
