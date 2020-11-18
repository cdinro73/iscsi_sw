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
#include <errno.h>

/*Initiator codes - RFC 5048 Page 27, , detailed in RFC 3720*/
#define NOPOUT          0  //NOP-In/NOP-Out: ping mechanism between initiator and target: verify that connection is up.
#define SCSI_REQ        1  //PDU containing SCSI command (SCSI payload only).
#define TASK_REQ        2  //PDU used to explicitly control the execution of one or more SCSI Tasks.
#define LOGIN_REQ       3  //Logout Requests and Responses PDUs used in the final phase for nicely closing connection.
#define TEXT_REQ        4  //Text requests and responses (key=value) for parameters in the negociation phase.
#define WRDATA          5  //SCSI Data-In PDU. Contains data to be written. 
#define LOGOUT          6  //Logout Requests and Responses used in the final phase for nicely closing connection.
#define SNACK           16 //SNACK Request: initiator requests retransmission.
#define INIT_SPEC1      28 //INIT_SPEC1-3 are vendor specific codes for initiator.
#define INIT_SPEC2      29 
#define INIT_SPEC3      30 


/*Target codes - RFC 5048 Page 26, detailed in RFC 3720*/
#define NOPIN           32 //NOP-In/NOP-Out: ping mechanism between initiator and target: verify that connection is up.
#define SCSI_RESP       33 //PDU containing SCSI response to a SCSI command.
#define TASK_RESP       34 //Response PDU for a task request PDU: indicates the completion of a task with a qualifier.    
#define LOGIN_RESP      35 //Login Requests and Responses are used exclusively during the Login Phase. 
#define TEXT_RESP       36 //Text requests and responses (key=value) for parameters in the negociation phase.    
#define RDATA           37 //SCSI Data-Out PDU. Contains data that has been readed.   
#define LOGOUT_RESP     38 //Logout Requests and Responses used in the final phase for nicely closing connection.
#define R2T             49 //Ready to Transfer. SCSI target announces the initiator that output data is comming.
#define ASYNC           50 //Asynchronous Messages are used to carry SCSI asynchronous events (AEN).
#define REJECT          63 //Reject is used to indicate an iSCSI error condition.
#define TARGET_SPEC1    60 //TARGEt_SPEC1-3 are vendor specific codes for target.
#define TARGET_SPEC2    61
#define TARGET_SPEC3    62


#define BHS 48		/* The length of the basic header segment: allways 48 bytes.*/
#define PORT 3260	/* iSCSI target listening port 3260 */
#define MAX  10240
#define LINE 80

typedef struct bhsHeader {
  u_char OPCode:6;	/* initiator/target code: unsigned char 6 bits */
  u_char Imm:1;		/* The immediate flag: indicates immediate delivery */
  u_char pad1:1;	/* 0 padding bit to fill the first octet of the BHS */
  u_char pad2:7;	/* 7 bites of 0 for padding the second octet of the BHS */
  u_char Final:1;	/* The Final flag: if set, indicates that PDU is the last in sequence */
  u_char pad3[2];	/* These 2 octets are not used (reserved). All bits are 0. */
  u_int AHSLen:8;	/* Additional Header Segments (AHS) length: AHS contains optional Header-Digest, and/or Data-Digest */
  u_int DSLen:24;	/* Data segment length: the length of the PDU payload */
  u_int Lun[2];		/* The Logical Unit Number in the target for which the PDU is addressed (can be empty) */
  u_int Itt;		/* Each PDU belongs to a task: initiator assigns a Task Tag to each iSCSI task it issues. */ 
  u_int OPCodeSpecFields[7]; 
} bhs_hdr;


void printBhs(bhs_hdr *bhs, const char* bhsName);	//Print BHS fields
void debugBhs(bhs_hdr *bhs, const char* bhsName);	//Print BHS fields in more details
/*Log BHS in more details and debug messages to log server*/
void logBhs(int log_fd, bhs_hdr *bhs, const char* bhsName, const char* initiatorIP, const char* targetIP);
void bhsToBuf(bhs_hdr* bhs, u_char* buf);		//Copy the 48 bytes BHS to a buffer
u_int dataLength(bhs_hdr* bhs);				//Calculate the data length of an iSCSI PDU
u_int getTextOpCode(bhs_hdr* bhs, char* buf);		//Extract the iSCSI Opcodes RFC 5048 - Page 25



