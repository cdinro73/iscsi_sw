CC = gcc
CFLAGS = -g -O2 -D_REENTRANT -Wall -w -I../include $(shell mysql_config --cflags) $(shell mysql_config --libs)
LIB_OBJS = sock.o byte.o scsi.o regexp.o lookup.o

CLEANFILES = core core.* *.core *.o temp.* router logsrv test_client routercli 

all:	${LIB_OBJS}
	${CC} ${CFLAGS} ${LIB_OBJS} router.c -o router
	${CC} ${CFLAGS} sock.c logsrv.c -o logsrv
	${CC} ${CFLAGS} routercli.c -o routercli
clean:
	rm -f ${PROGS} ${CLEANFILES}

