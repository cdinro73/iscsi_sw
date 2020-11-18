CC = gcc
CFLAGS = -g -O2 -D_REENTRANT -Wall -I../include $(shell mysql_config --cflags) $(shell mysql_config --libs)

CLEANFILES = core core.* *.core *.o temp.* router logsrv test_client routercli 

all:	
	${CC} ${CFLAGS} ${LIB_OBJS} lookup2.c -o lookup

clean:
	rm -f ${PROGS} ${CLEANFILES}

