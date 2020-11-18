#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
 
typedef struct
{
    u_char bit0:1; //first bit (data storage of size 1)
    u_char bit1:1;
    u_char bit2:1;
    u_char bit3:1;
    u_char bit4:1;
    u_char bit5:1;
    u_char bit6:1;
    u_char bit7:1;
} byte_t;


void printByteStruct(byte_t structByte);		//Print the byte_t structure bit by bit
void printByte(u_char byte, const char* byteName);	//Print a byte bit by bit
