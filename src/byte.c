#include "byte.h"
 
/* Print the byte_t structure bit by bit */
void printByteStruct(byte_t structByte)
{
        printf ("%d %d %d %d %d %d %d %d  ",
            structByte.bit0, structByte.bit1, structByte.bit2, structByte.bit3,
            structByte.bit4, structByte.bit5, structByte.bit6, structByte.bit7);
}

/* Print a byte bit by bit */
void printByte(u_char byte, const char* byteName)
{
	byte_t structByte;
	structByte = *((byte_t *) (&byte));
	printf("%s", byteName);
	printByteStruct(structByte);
}


/*
int main()
{
    u_char byte;
    ucbyte = 0x00;
    printByte(byte);

    byte = 0xFF;
    printByte(byte, "Test byte");

    return 0;
}
*/
