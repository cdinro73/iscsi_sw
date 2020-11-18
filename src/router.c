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
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <errno.h>
#include "scsi.h"
#include "byte.h"
#include "regexp.h"
#include "sock.h"

char targetAddr[16]; //the IP address of the iSCSI target server
u_int targetPort; //the port to connect to the target
char nextHopIP[16]; //local IP to bind
u_int nextHopPort; //the socket port at the local endpoint
char *logIP; //the IP address of the remote log server
int childCnt = 0;  //counts the number of forks

void
Router(int s1, int s2) //s1 is the socket to initiator, s2 is the socket to target
{
	struct pollfd ufds[2];
	int rv;
	char buf[MAX];
	char tracerAddr[40];
	char initiatorAddr[40];
	char tgtAddr[40];
	int nbytes, dslen, n, log_fd;	
	bhs_hdr *bhs = (bhs_hdr*)malloc(sizeof(bhs_hdr));

        ufds[0].fd = s1;
        ufds[0].events = POLLIN; // check for incomming data on the socket s1

        ufds[1].fd = s2;
        ufds[1].events = POLLIN; // check for incomming data on the socket s2

	getSockIP(s1, tracerAddr, 40); //retrieve the IP address of the endpoint for s1
	getPeerIP(s1, initiatorAddr, 40); //retreive the IP address of the initiator	
	getPeerIP(s2, tgtAddr, 40); //retreive the IP address of the target

	printf("tracerAddr: %s initiatorAddr: %s tgtAddr: %s\n", tracerAddr, initiatorAddr, tgtAddr);

	if (logIP == NULL)
        log_fd = -1;

        else if( ( log_fd = logSrvConnect(logIP) ) < 0 )//connect to log server
        {
               printf("Could not connect to log server\n");
        }
        // wait for events on the itwo sockets, timeout 3 second
        while(1)
        {
		bzero(buf, MAX);
                rv = poll(ufds, 2, 3000);
                if (rv == -1)
                        perror("poll"); // error occurred in poll()
                else if (rv == 0)
		{
                        //printf("Timeout occurred. No data received in 3 seconds.\n");
		}
                else
                {
                        // check for events on s1: the initiator socket
                        if (ufds[0].revents & POLLIN)
                        {
                                //cdinu
				nbytes = recvn(s1, bhs, sizeof(bhs_hdr)); //read the basic header struct from initiator socket
				if(nbytes == 0) break;
				//printf("Child %d read BHS %d\n", getpid(), nbytes);
				/* logBhs and send messages to the log server */
				if(log_fd > 0)
					logBhs(log_fd, bhs, "initiator", initiatorAddr, tgtAddr);
				/* end logging */
				dslen = dataLength(bhs); //calculate the length of the PDU payload
				if(dslen)
				{
					//printf("Waiting %d data in recvn from initiator\n", dslen);
                                	nbytes = recvn(s1, buf, dslen);//read the rest of the PDU from initiator socket
					//printf("Expected: %d Received %d\n", dslen, nbytes);
					//return;
					if( nbytes > 0)
					{
						//printf("Send to server: %d\n", nbytes + 48);
						//send the PDU to the target
						sendn(s2, bhs, sizeof(bhs_hdr)); //first send the BHS
						sendn(s2, buf, nbytes);//Now send the payload
					}
					else 
						break;
                                }

				else
				{
					//If PDU has no payload, send only the BHS
					//printf("Send to server: 48\n");
					sendn(s2, bhs, sizeof(bhs_hdr));
				}


                        }
                        // check for events on s2: the target socket
                        if (ufds[1].revents & POLLIN)
                        {
                                //cdinu
				nbytes = recvn(s2, bhs, sizeof(bhs_hdr));//read the basic header struct from the target socket
				if(nbytes == 0) break;
				//printf("Child %d read BHS %d\n", getpid(), nbytes);
				/* logBhs and send messages to the log server */
                                if(log_fd > 0)
                                        logBhs(log_fd, bhs, "target", initiatorAddr, tgtAddr);
				/* end logging */
				dslen = dataLength(bhs);// calculate the length of the payload
                                if(dslen)
                                {
					//printf("Waiting %d data in recvn from server\n", dslen);
					bzero(buf, MAX);
                                        nbytes = recvn(s2, buf, dslen);//receive the rest of the PDU
					/* Tracer is acting like iSCSI target. Replace the target IP with the IP of tracer */
  					if(bhs->OPCode == TEXT_RESP)
					{
						strReplace(buf, tgtAddr, tracerAddr);
                                                for(n = strlen(buf); n< nbytes; n++)
	                                                buf[n] = '\0';
						bhs->DSLen += strlen(tracerAddr) - strlen(tgtAddr);
					}
					//printf("Expected: %d Received %d\n", dslen, nbytes);
                                        if( nbytes > 0)
                                        {
						//printf("Send bhs to initiator\n");
                                                sendn(s1, bhs, sizeof(bhs_hdr));
						//printf("Send data to initiator\n");
                                                sendn(s1, buf, nbytes);
						//printf("Am trimis\n");
                                        }
                                        else
                                                break;
                                }

                                else
                                {
					//If PDU has no payload (it's a BHS) then send the BHS only
					//printf("Send 48 to client %s from pid: %d\n", initiatorAddr, getpid());
                                        sendn(s1, bhs, sizeof(bhs_hdr));
                                }




                        }

                }
       }
}

void chldExitHandler(int sig) 
{
	pid_t pid;
	int stat;

	/* loop as long as there are children to process */
	while (1) 
	{
		/* retrieve child process ID (if any) */
		pid = waitpid(-1, &stat, WNOHANG);

		/* check for conditions causing the loop to terminate */
		if (pid == -1) 
		{
			if (errno == EINTR) 
        			continue;
           		break;
       		}
       		else if (pid == 0) 
           		/* no more children to process, so break */
           		break;
        	else
           		childCnt--;
	}
}



int main(int argc, char *argv[])
{
        int                     listenfd, dst_fd, src_fd;
        struct sockaddr_in      serv_addr;
        const int on = 1;
	pid_t pid;
	char initiatorAddr[40];  
	char charInitPort[8];
	u_int initPort;

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = chldExitHandler;
	sigaction(SIGCHLD, &sa, NULL);


	//parse command line arguments to extract IP address of target
	int opt = 0;
	char *listenIP = NULL;
	logIP = NULL;
	while ((opt = getopt(argc, argv, "l:d:")) != -1) 
	{
    		switch(opt)
    		{   
                case 'l':
                   listenIP = optarg;
                   break;

		case 'd':
		   logIP = optarg;
		   break;

         	case '?':
                  if (optopt == 'l')
                        printf("\nMissing listening interface");
		  else if (optopt == 'd')
			printf("\nMissing log server IP");
		  else 
		  {
     			printf("\nInvalid option received\n");
			return -1;
		  }	
		 
        	}
	}


	/*create listening socket for accepting connections*/
	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creating socket %s", strerror(errno));
	if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		printf("Error setsockopt %s", strerror(errno));
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	if(listenIP)
		if( inet_pton(AF_INET, listenIP, &serv_addr.sin_addr) <= 0 )
		{
			printf("Invalid IP for listen interface: %s", strerror(errno));
			return -1;
		}
		else
		{
			printf("Listen IP: %s\n", listenIP);
		}
	else
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT); 
	if( bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 )
		printf("Error binding socket %s", strerror(errno)); 
	if( listen(listenfd, 10) < 0 )
		printf("Error listening %s", strerror(errno));

	/*Accepting connections*/
	while(1)	
	{
        	src_fd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        	//ioctl(src_fd, (int)FIONBIO, (char *)&on);
		if(src_fd < 0)
		{
			printf("Error accept");
			continue;
		}

		//Connection from initiator accepted. Now fork to serve the initiator.

		pid = fork();
		if(pid == -1)// fork failed
		{
			printf("Error fork\n");
			return -1;
		}

		else if(pid == 0) //child
		{
			close(listenfd);//listen socket must be closed
			getPeerIP(src_fd, initiatorAddr, sizeof(initiatorAddr));
			getPeerPort(src_fd, charInitPort, sizeof(charInitPort));
			initPort = atoi(charInitPort);
			if( tableLookup(targetAddr, &targetPort, initiatorAddr, initPort, nextHopIP, &nextHopPort) < 0 )
                	{
                        	printf("No route found\n");
                        	return -1;
                	}
                	else
                        	printf("Target for %s: %s\n", initiatorAddr,targetAddr);

                	if( ( dst_fd = targetConnect(targetAddr, targetPort, nextHopIP, nextHopPort) ) < 0 )//connect to iSCSI target
                	{
                        	printf("Could not connect to target\n");
                        	return -1;
                	}

			Router(src_fd, dst_fd);
			close(src_fd);
			close(dst_fd);
			return 0;
		}
			
		//parent	
		close(src_fd);
		printf("\nChild spawned with pid:  %d",pid);
		childCnt++;
	}
	close(listenfd);//Before finish, close listen socket	
	return 0;
}

