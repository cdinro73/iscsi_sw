#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include "sock.h"

#define MAX 1024
#define MAXCL 20 //max bumber of Gui clients
#define TRACER "tracer client"//message that identifies gui client
#define LOG_NAME "Tracer"


void sysLog(char *message)
{
        openlog (LOG_NAME, LOG_NDELAY, LOG_LOCAL0);
        if(strcasestr(message, "Error"))
                syslog (LOG_ERR, "%s", message);
        else
                syslog (LOG_INFO, "%s", message);
        closelog ();
}



int main()
{
    int listenFd, connFd; //listen and connected socket
    int tracerFd[MAXCL]; //file descriptor array of the tracer client
    int maxFd, maxIdx; // max descriptor and max index
    int i, l, m, n, on = 1;
    fd_set tmpSet, currentSet;  // monitored descriptor set
    int fdArray[FD_SETSIZE], Ready; // file descriptors array from client connections
    struct sockaddr_in srvAddr, clientAddr;
    char buff[MAX];

    /* initialization */
    for(i = 0; i<MAXCL; i++)
	tracerFd[i] = -1;
  
    listenFd = socket(AF_INET, SOCK_STREAM, 0);

    if(listenFd < 0)
        printf("Failed creating socket\n");

    /* preparing internet address structure */
    bzero((char *)&srvAddr, sizeof(srvAddr));

    srvAddr.sin_family = AF_INET;
    srvAddr.sin_addr.s_addr = INADDR_ANY;
    srvAddr.sin_port = htons(1973);

    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    /* bind listening socket */
    if (bind(listenFd, (struct sockaddr *)&srvAddr, sizeof(srvAddr)) < 0)
        printf("Failed to bind\n");

    listen(listenFd, 5);

    maxFd = listenFd; // Max descriptor initialization with the listen file descriptor
    maxIdx = -1; // index the array client connected descriptors
    for (l=0; l<FD_SETSIZE; l++)
        fdArray[l] = -1;  // -1 means the entry is available to be associated with a valid descriptor
    FD_ZERO(&currentSet); // empty the monitored descriptor set (initialization)
    FD_SET(listenFd, &currentSet); // listening descriptor should be monitored.

    while(1) // loop
    {
	/* Save the monitored descriptor set in tmpSet: select function will overwrite the currentSet. */
        tmpSet = currentSet;
        Ready = select(maxFd+1, &tmpSet, NULL, NULL, NULL); // select will watch tmpSet for activity, with maxFd

	/* new connection to the listening socket */
        if(FD_ISSET(listenFd, &tmpSet)) 
        {
            printf("New connection\n");
            u_int size = sizeof(clientAddr);
            connFd = accept(listenFd, (struct sockaddr *)&clientAddr, &size);
	    //ioctl(connFd, (int)FIONBIO, (char *)&on);
            for (m=0; m<FD_SETSIZE; m++)
                if(fdArray[m] < 0)
                {
                    fdArray[m] = connFd; // Add new connected socket descriptor to the array of file descriptors
                    break;
                }

                FD_SET(connFd, &currentSet); // The new descriptor will be monitored (added to monitored set)
		/* new max election for maxFd and maxIdx */
                if(connFd > maxFd)
                    maxFd = connFd; 
                if(m > maxIdx)
                    maxIdx = m;
		if(--Ready <=0) 
                    continue;					
        }

	/* Check all client file descriptors to see if data available */
        for(n=0; n<=maxIdx; n++)
        {
            if(fdArray[n] > 0)
            {
                if(FD_ISSET(fdArray[n], &tmpSet))
                {
		    int nbytes;
                    if( (nbytes = read(fdArray[n], buff, MAX)) > 0)
                    {

			buff[nbytes] = '\0';
                        printf("%d %s", nbytes, buff);
			sysLog(buff);
			if(strstr(buff,TRACER))
			{
				printf("This is tracer gui client\n");
				/* ad descriptor to tracerFd array */
				for(i=0; i< MAXCL; i++)
				   if(tracerFd[i] <0)
				   {
				      tracerFd[i] = fdArray[n];
				      break;
				   }
			}
			else
			{       /* send message to all gui clients */
				for(i=0; i< MAXCL; i++)
		                   if(tracerFd[i] > 0)
				     write(tracerFd[i], buff, nbytes);
			}

                    }

		    /* 0 bytes readed from socket means connection was closed by client */
                    if(nbytes == 0)  
                    {
			/* First remove also the descriptor for the gui array */
			for(i=0; i< MAXCL; i++)
			   if(tracerFd[i] == fdArray[n])
				tracerFd[i] = -1;
                        close(fdArray[n]);
                        FD_CLR(fdArray[n], &currentSet);
			/* At the end, set fdArray[n] to -1 */
                        fdArray[n] = -1;
                    }
		    /* No more monitoring descriptors */
                    if(--Ready <=0) 
                        break; 
                }
            }
        }
    } // end of loop

    close(listenFd);
    return 0;
}


