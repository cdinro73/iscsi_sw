#include "scsi.h"
#include "byte.h"


/* Calculate the data length of an iSCSI PDU */
u_int dataLength(bhs_hdr* bhs)
{
	u_int n;
	ssize_t dslen;
	n = bhs->DSLen;
        dslen = ( (n & 0x00ff0000) >> 16 ) | (n & 0x0000ff00) | ( (n & 0x000000ff) << 16 );
	if(bhs->AHSLen)
        	dslen += 4*bhs->AHSLen; //four bytes words RFC 3720 pag 115
	while(dslen & 3) dslen++; //3 is 1100:0000
	//printf("From dataLength AHS: %d BHS: %d dslen:%d\n", bhs->AHSLen, bhs->DSLen, dslen);
	return dslen;
}
 
/* Extract the iSCSI Opcodes RFC 5048 - Page 25 */
u_int getTextOpCode(bhs_hdr* bhs, char* buf)
{
	switch(bhs->OPCode)
	{	
		//Initiator
		case NOPOUT:  
			strcpy(buf, "Initiator, NOP-Out");
			break;        
		case SCSI_REQ:     
			strcpy(buf, "Initiator, SCSI Command");
			break;  
		case TASK_REQ:
			strcpy(buf, "Initiator, SCSI Task Management function request");
			break;        
		case LOGIN_REQ:
			strcpy(buf, "Initiator, Login Request");
			break;       
		case TEXT_REQ:
			strcpy(buf, "Initiator, Text Request");
			break;        
		case WRDATA:  
			strcpy(buf, "Initiator, SCSI Data-Out");
			break;        
		case LOGOUT:  
			strcpy(buf, "Initiator, Logout Request");
			break;        
		case SNACK:   
			strcpy(buf, "Initiator, SNACK Request");
			break;
		case INIT_SPEC1:
        		strcpy(buf, "Initiator, Vendor specific code 0x1c");
        		break;	
		case INIT_SPEC2:
        		strcpy(buf, "Initiator, Vendor specific code 0x1d");
        		break;
		case INIT_SPEC3:
        		strcpy(buf, "Initiator, Vendor specific code 0x1e");
        		break;
		//Target
		case NOPIN:
        		strcpy(buf, "Target NOP-In");
        		break;
		case SCSI_RESP:
        		strcpy(buf, "Target, SCSI Response");
        		break;
		case TASK_RESP:
        		strcpy(buf, "Target, SCSI Task Management function response");
        		break;
		case LOGIN_RESP:
        		strcpy(buf, "Target, Login Response");
        		break;
		case TEXT_RESP:
        		strcpy(buf, "Target, Text Response");
        		break;
		case RDATA:
        		strcpy(buf, "Target, SCSI Data-In");
        		break;
		case LOGOUT_RESP:
        		strcpy(buf, "Target, Logout Response");
        		break;
		case R2T:
        		strcpy(buf, "Target, Ready To Transfer");
        		break;
		case ASYNC:
        		strcpy(buf, "Target, Asynchronous Message");
        		break;
		case REJECT:
        		strcpy(buf, "Target, Reject");
        		break;
		case TARGET_SPEC1:
        		strcpy(buf, "Target, Vendor specific code 0x3c");
        		break;	
		case TARGET_SPEC2:
        		strcpy(buf, "Target, Vendor specific code 0x3d");
        		break;
		case TARGET_SPEC3:
        		strcpy(buf, "Target, Vendor specific code 0x3e");
        		break;
		default:
			strcpy(buf, "Error: not in opcodes");
	}

	return bhs->OPCode;

}

/* Print BHS fields */
void printBhs(bhs_hdr *bhs, const char* bhsName)
{
	printf("BHS from %s:\n", bhsName);
	printByte(bhs->OPCode, "OPCode");
	printf("OPCODE: %d\n", bhs->OPCode);
	printByte(bhs->Imm, "The Immediate flag Imm");
	printByte(bhs->Final, "The Final flag");

	printf("AHS Length: %d\n", bhs->AHSLen);
	printf("DSLen: %d\n", bhs->DSLen);
	printf("Lun ID: %d %d\n", bhs->Lun[0], bhs->Lun[1]);
	printf("Itt: %d\n", bhs->Itt);
	printf("CmdSN: %d\n", ntohl(bhs->OPCodeSpecFields[1]));
	printf("ExpStatSN: %d\n", ntohl(bhs->OPCodeSpecFields[2]));
	printf("MaxCmdSN: %d\n", ntohl(bhs->OPCodeSpecFields[3]));
}

/* Copy the 48 bytes BHS to a buffer */
void bhsToBuf(bhs_hdr* bhs, u_char* buf)
{
	memcpy(buf, bhs, BHS); 	
}

/* Print BHS fields in more details */
void debugBhs(bhs_hdr *bhs, const char* bhsName)
{
        int i,k;
        char txtOpCode[LINE];
        u_int opCode;
        printf("%s\n", bhsName);
        u_char buf[48];
        bhsToBuf(bhs, buf);
        opCode = getTextOpCode(bhs, txtOpCode);
        printf("------------------------------\n");
        for(k=0; k<12; k++)
        {
                for(i = 4*k; i<4*(k+1); i++)
                        printByte(buf[i],"");
                printf("\n");
        }
        printf("BHS from %s:\n", bhsName);
        printByte(bhs->OPCode, "OPCode");
        printf("OPCODE: %d %s\n", opCode, txtOpCode);
        printByte(bhs->Imm, "The Immediate flag Imm");
        printByte(bhs->Final, "The Final flag");
        printf("Data Length: %d\n", dataLength(bhs));
        printf("Lun ID: %d %d\n", ntohl(bhs->Lun[0]), ntohl(bhs->Lun[1]));
        printf("Itt: %d\n", ntohl(bhs->Itt));
        printf("CmdSN: %d\n", ntohl(bhs->OPCodeSpecFields[1]));
        printf("ExpStatSN: %d\n", ntohl(bhs->OPCodeSpecFields[2]));
        printf("MaxCmdSN: %d\n", ntohl(bhs->OPCodeSpecFields[3]));
        printf("------------------------------\n");
}

/* This function will send the log message to log server */
void logBhs(int log_fd, bhs_hdr *bhs, const char* bhsName, const char* initiatorIP, const char* targetIP)
{
        char sendBuf[1024];//data that will be sent to log server socket log_fd
        char txtOpCode[200];
	char tmp[300];
        u_int opCode;
        opCode = getTextOpCode(bhs, txtOpCode);

	if(strcmp(bhsName, "initiator")) //BHS is from initiator to target
		sprintf(tmp, "Initiator %s -> target    %s", initiatorIP, targetIP);
	else if(strcmp(bhsName, "target")) //BHS is from target to initiator
		 sprintf(tmp, "Target   %s -> initiator %s", targetIP, initiatorIP);

        sprintf(sendBuf, "%s\tOpcode: %02x %-30s\t Data length: %-4d\tFinal Flag: %d\tImm Flag: %d\tItt: %d\n", 
			 tmp, opCode, txtOpCode, dataLength(bhs), bhs->Final, bhs->Imm, bhs->Itt);
	write(log_fd, sendBuf, strlen(sendBuf));
	
}

