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

	if(argc ==1 ){
		return;
	}


	msqid = ku_msgget(atoi(argv[1]), IPC_CREAT);
	// 	msqid = ku_msgget(1, IPC_EXCL);
	printf("msqid(0)? : %d\n", msqid);
	ret = ku_msgrcv(msqid, &msg_buf, KUIPC_MAXMSG, TYPE, IPC_NOWAIT);
	//ku_msgrcv(msqid, &msg_buf, KUIPC_MAXMSG, TYPE, IPC_NOERROR);
	
	printf("received msg : %s, type[%ld] \n", msg_buf.text, msg_buf.type);
}
