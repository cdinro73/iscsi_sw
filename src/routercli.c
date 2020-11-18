#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>

#define MAXMSG 1024		//max length for received messages
#define PORT 1973 		//this is the port of log server
#define TRACER "tracer client"	//message that identifies gui client

void Err(char *msg);	//wrapper for eroneous situations

int main(int argc, char *argv[]) {
    int listenFd;
    struct sockaddr_in logAddr;
    struct hostent *srvStruct;
    char *logIP;
    char buf[MAXMSG];
    int nbytes;
    int opt = 0;

    /* parse command line arguments for logIP*/
    logIP = NULL;
    if(argc < 2)
    {
         printf("\nUsage: -d <Log_IP>\n");
         return -1;
    }
    while ((opt = getopt(argc, argv, "d:")) != -1)
    {
         switch(opt)
         {
             case 'd':
             	logIP = optarg;
             	break;

             case '?':
                if (optopt == 'd')
		{
                        printf("\nMissing log server IP");
			return -1;
		}
                else
                {
                        printf("\nInvalid option received\n");
                        return -1;
                }

          }
    }


    /* Create the listening socket listenFd */
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
        Err("Err opening socket");

    /* gethostbyname for DNS entry */
    srvStruct = gethostbyname(logIP);
    if (srvStruct == NULL) {
        fprintf(stderr,"Err, no such host as %s\n", logIP);
        exit(0);
    }

    /* initialize the Internet address structure */
    bzero((char *) &logAddr, sizeof(logAddr));
    logAddr.sin_family = AF_INET;
    bcopy((char *)srvStruct->h_addr, (char *)&logAddr.sin_addr.s_addr, srvStruct->h_length);
    logAddr.sin_port = htons(PORT);

    /* Connect to server */
    if (connect(listenFd, (struct sockaddr *)&logAddr, sizeof(logAddr)) < 0)
      Err("Err connecting");

    /* Send tracer gui identifier to server */
    nbytes = write(listenFd, TRACER, strlen(TRACER));
    if (nbytes < 0)
      Err("Err write to socket");

    /* read messages */
    while(1)
    {
    	bzero(buf, MAXMSG);
	nbytes = read(listenFd, buf, MAXMSG);
	if (nbytes < 0)
             Err("Err read from socket: \n");
	buf[nbytes] = '\0';
	printf("%s", buf);
	
    }
    
    close(listenFd);
    return 0;

}

void Err(char *msg) {
    printf("%s %s\n", msg, strerror(errno));
    exit(-1);

}
