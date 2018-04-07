#include <stdio.h>
#include <string.h>

#include "ku_ipc.h"

#define TYPE 0

void main(int argc, char* argv[]){
	int key;
	char msg[KUIPC_MAXMSG];
	struct msgbuf msg_buf;
	int msqid;
	int i=0;
	msqid = atoi(argv[2]);
	printf("%d\n",msqid);
	
	if(msqid == -1){
		printf("already exist key : %d", key);
		return;
	}

	if(ku_msgclose(msqid) == -1){
		printf("failed to msgclose, msqid[%d] is not exist\n", msqid);
	} else {
		printf("success to msgclose\n");
	}
}
