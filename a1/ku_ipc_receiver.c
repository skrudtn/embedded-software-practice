#include <stdio.h>
#include <string.h>

#include "ku_ipc.h"

#define TYPE 0
void main(int argc, char* argv[]){
	int key;
	char msg[KUIPC_MAXMSG];
	struct msgbuf msg_buf;
	int msqid;
	int ret;
	long type;

	if(argc ==1 ){
		return;
	}


	//msqid = ku_msgget(atoi(argv[1]), IPC_CREAT);
	msqid = 1;
	// 	msqid = ku_msgget(1, IPC_EXCL);
	type = atol(argv[1]);
	printf("%ld\n", type);
	ret = ku_msgrcv(msqid, &msg_buf, KUIPC_MAXMSG, type, IPC_NOWAIT);
	//ku_msgrcv(msqid, &msg_buf, KUIPC_MAXMSG, TYPE, IPC_NOERROR);
	
	printf("received msg : %s, type[%ld] \n", msg_buf.text, msg_buf.type);
	printf("remove queue : %d", msqid);
//	ku_msgclose(msqid);
}
