#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define SCSI_PORT 3260
#define LOG_PORT 1973

/* Read exactly n bytes from the socket fd and store them in Buff */
ssize_t recvn(int fd, void *Buff, size_t n);

/* Writes exactly n bytes to the socket fd from Buff */
ssize_t sendn(int fd, const void *Buff, size_t n);

/* Read null terminated strings from socket fd */
ssize_t readString(int fd, void *Buff, size_t n);

/* Return the IP:PORT address of the remote client connected to the TCP server with socket s*/
char * getPeerAddr(int s, char *buffer, size_t bSize);

/* Return the IP address to which the socket s is bound */
char * getSockAddr(int s, char *buffer, size_t bSize);

/* Connect to the remote iSCSI target TCP server */
int targetConnect(char* targetAddr, u_int targetPort, char* nextHopIP, u_int nextHopPort);

/* Connect to the remote logging server */
int logSrvConnect(char *logIP);
