#include "sock.h"

/* Read exactly n bytes from the socket fd and store them in Buff */
ssize_t recvn(int fd, void *Buff, size_t n)
{
    ssize_t curRead;                    
    size_t countRead;                    
    char *buffer;

    buffer = Buff;                       
    for (countRead = 0; countRead < n; ) {
        curRead = read(fd, buffer, n - countRead);

        if (curRead == 0)               
            return countRead;            
        if (curRead == -1) {
            if (errno == EINTR)
                continue;               
            else
                return -1;              
        }
        countRead += curRead;
        buffer += curRead;
    }
    return countRead;                     
}

/* Writes exactly n bytes to the socket fd from Buff */
ssize_t sendn(int fd, const void *Buff, size_t n)
{
    ssize_t curWr;                 
    size_t countWr;                 
    const char *buffer;

    buffer = Buff;                       
    for (countWr = 0; countWr < n; ) {
        curWr = write(fd, buffer, n - countWr);

        if (curWr <= 0) {
            if (curWr == -1 && errno == EINTR)
                continue;               
            else
                return -1;              
        }
        countWr += curWr;
        buffer += curWr;
    }
    return countWr;                 
}

/* Read null terminated strings from socket fd */
ssize_t readString(int fd, void *Buff, size_t n)
{
    ssize_t curRead;                    
    size_t countRead;                    
    char *buffer;
    char ch;

    if (n <= 0 || Buff == NULL) 
    {
        errno = EINVAL;
        return -1;
    }

    buffer = Buff;                       

    countRead = 0;
    while(1)
    {
        curRead = read(fd, &ch, 1);

        if (curRead == -1) 
	{
            if (errno == EINTR)         
                continue;
            else
                return -1;              

        } 
	else if (curRead == 0) 
	{      
            if (countRead == 0)           
                return 0;
            else                        
                break;

        } 
	else 
	{                        
            if (countRead < n - 1) 
	    {
                countRead++;
                *buffer++ = ch;
            }

            if (ch == '\0')
                break;
        }
    }

    *buffer = '\0';
    return countRead;
}


/* Return the IP:PORT address of the remote client connected to the TCP server with socket s*/
char * getPeerAddr(int s, char *buffer, size_t bSize) {
    int g; 
    struct sockaddr_in inetAddr;
    u_int inetLength;

    inetLength = sizeof inetAddr;

    g = getpeername(s, (struct sockaddr *)&inetAddr, &inetLength);
    if ( g == -1) {
       return NULL; 
    }

    g = snprintf(buffer,bSize, "%s:%u",  inet_ntoa(inetAddr.sin_addr), (unsigned)ntohs(inetAddr.sin_port));
    if ( g == -1 ) {
       return NULL; 
    }
    return buffer;
 }

/* Return the IP address of the remote client connected to the TCP server with socket s*/
char * getPeerIP(int s, char *buffer, size_t bSize) {
    int g;
    struct sockaddr_in inetAddr;
    u_int inetLength;

    inetLength = sizeof inetAddr;

    g = getpeername(s, (struct sockaddr *)&inetAddr, &inetLength);
    if ( g == -1) {
       return NULL;
    }

    g = snprintf(buffer,bSize, "%s",  inet_ntoa(inetAddr.sin_addr));
    if ( g == -1 ) {
       return NULL;
    }
    return buffer;
 }

/* Return the PORT address of the remote client connected to the TCP server with socket s*/
char * getPeerPort(int s, char *buffer, size_t bSize) {
    int g;
    struct sockaddr_in inetAddr;
    u_int inetLength;

    inetLength = sizeof inetAddr;

    g = getpeername(s, (struct sockaddr *)&inetAddr, &inetLength);
    if ( g == -1) {
       return NULL;
    }

    g = snprintf(buffer,bSize, "%u",  (unsigned)ntohs(inetAddr.sin_port));
    if ( g == -1 ) {
       return NULL;
    }
    return buffer;
 }

/* Return the IP address to which the socket s is bound */
char * getSockAddr(int s, char *buffer, size_t bSize) {
    int g;
    struct sockaddr_in inetAddr;
    u_int inetLength;

    inetLength = sizeof inetAddr;

    g = getsockname(s, (struct sockaddr *)&inetAddr, &inetLength);
    if ( g == -1) {
       return NULL;
    }

    //g = snprintf(buffer,bSize, "%s:%u",  inet_ntoa(inetAddr.sin_addr), (unsigned)ntohs(inetAddr.sin_port));
    g = snprintf(buffer,bSize, "%s",  inet_ntoa(inetAddr.sin_addr));
    if ( g == -1 ) {
       return NULL;
    }
    return buffer;
 }

/* Return the IP address to which the socket s is bound */
char * getSockIP(int s, char *buffer, size_t bSize) {
    int g;
    struct sockaddr_in inetAddr;
    u_int inetLength;

    inetLength = sizeof inetAddr;

    g = getsockname(s, (struct sockaddr *)&inetAddr, &inetLength);
    if ( g == -1) {
       return NULL;
    }

    g = snprintf(buffer,bSize, "%s",  inet_ntoa(inetAddr.sin_addr));
    if ( g == -1 ) {
       return NULL;
    }
    return buffer;
 }

/* Connect to the remote iSCSI target TCP server */
int targetConnect(char* targetAddr, u_int targetPort, char* nextHopIP, u_int nextHopPort)
{
        int                     dst_fd, len;
        struct sockaddr_in      address;//target endpoint
	struct sockaddr_in	local_addr;//local endpoint
	struct hostent 		*srvStruct;
        const int on = 1;

        //Create socket for client.
        if( (dst_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("Error creating socket to target: %s\n", strerror(errno));
		return -1;
	}

	srvStruct = gethostbyname(targetAddr);
	if (srvStruct == NULL) 
	{
        	fprintf(stderr,"Err, no such host as %s\n", targetAddr);
        	return -1;
    	}

        //build internet addresses and bind, then connect
	//bind to local endpoint
	if(nextHopPort > 0)
	{
   	   memset(&local_addr, 0, sizeof(struct sockaddr_in));
   	   local_addr.sin_family = AF_INET;
   	   local_addr.sin_port = htons(nextHopPort);
   	   local_addr.sin_addr.s_addr = inet_addr(nextHopIP);

	   if( setsockopt(dst_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
                printf("Error setsockopt %s", strerror(errno));

   	   if (bind(dst_fd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) < 0)
   	   {
       	     printf("Error bind to local ip and port %s:%d %s", nextHopIP, nextHopPort, strerror(errno));
             return -1; 
   	   }
         }


	bzero((char *) &address, sizeof(address));
        address.sin_family = AF_INET;
	bcopy((char *)srvStruct->h_addr, (char *)&address.sin_addr.s_addr, srvStruct->h_length);
        address.sin_port = htons(targetPort);
        len = sizeof(address);
        if( connect(dst_fd, (struct sockaddr *)&address, len) < 0 )
	{
		printf("Error connecting to target: %s", strerror(errno));
		return -1;
	}
        //ioctl(dst_fd, (int)FIONBIO, (char *)&on);
        return dst_fd;

}

/* Connect to remote log server to log PDUs */
int logSrvConnect(char *logIP)
{
        int                     fd, len;
        struct sockaddr_in      address;
	struct hostent          *srvStruct;
        //const int on = 1;

        //Create socket for client. Can be modified to be UDP
        if( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        {
                printf("Error creating socket To log server: %s\n", strerror(errno));
                return -1;
        }

        srvStruct = gethostbyname(logIP);
        if (srvStruct == NULL)
        {
                fprintf(stderr,"Err, no such host as %s\n", logIP);
                return -1;
        }

        //build internet address
        bzero((char *) &address, sizeof(address));
        address.sin_family = AF_INET;
        bcopy((char *)srvStruct->h_addr, (char *)&address.sin_addr.s_addr, srvStruct->h_length);
        address.sin_port = htons(LOG_PORT);
        len = sizeof(address);
        if( connect(fd, (struct sockaddr *)&address, len) < 0 )
        {
                printf("Error connecting to log server: %s", strerror(errno));
                return -1;
        }
        return fd;

}
