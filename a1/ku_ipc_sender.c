#include <stdio.h>
#include <string.h>

#include "ku_ipc.h"


void main(int argc, char* argv[]){
	int n = -1;
	int key;
	char msg[KUIPC_MAXMSG];
	struct msgbuf msg_buf;
	int msqid = 0;

	if(argc == 1){
		return;
	}


	msqid = ku_msgget(atoi(argv[1]), IPC_CREAT);
	// 	msqid = ku_msgget(1, IPC_EXCL);
	printf("msqid : %d\n", msqid);
	strncpy(msg,argv[2],sizeof(argv[2]));
	strncpy(msg_buf.text, msg, sizeof(msg));
	msg_buf.type = 1;
	printf("input msg : %s\n", msg);
	printf("buf text : %s\n",msg_buf.text);
	ku_msgsnd(msqid, &msg_buf, sizeof(msg_buf), IPC_NOWAIT);
}
