#include <stdio.h>
#include <unistd.h>

int main (int argc, char *argv[]) {
int opt = 0;
char *in_fname = NULL;
char *listenIP = NULL;

/*
        while ((opt = getopt(argc, argv, "s:l:")) != -1)
        {
                switch(opt)
                {  
                case 's':
                   targetIP = optarg;
                   break;

                case 'l':
                   listenIP = optarg;
                   printf("\nListen interface=%s", listenIP);
                   break;

                case '?':
                  if (optopt == 's')
                  {
                        printf("\nUsage: -s Target_IP (178.157.82.99)\n");
                        return -1;
                  }
                  else if (optopt == 'l')
                        printf("\nMissing listening interface");
                  else
                  {
                        printf("\nInvalid option received\n");
                        return -1;
                  }

                }
        }

*/


while ((opt = getopt(argc, argv, "s:l:")) != -1) {
	switch(opt) {

		case 's':
			in_fname = optarg;
			printf("\nTarget IP =%s", in_fname);
	        	break;
		case 'l':
			listenIP = optarg;
			printf("\nListen interface=%s", listenIP);
			break;
		case '?':
			if (optopt == 's') 
			{
				printf("\nMissing target IP");
			} 

			else if (optopt == 'l') 
			{
				printf("\nMissing listening interface");
			} 
			else 
			{
				printf("\nInvalid option received");
			}
			break;
		}
 }

printf("\n");
return 0;
 }

