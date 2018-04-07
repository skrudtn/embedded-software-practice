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
	key = 1;
	//msqid = ku_msgget(key, IPC_EXCL);
	msqid = ku_msgget(key, IPC_CREAT);
	type = atol(argv[1]);
	
	printf("%ld\n", type);
	printf("%d\n", msqid);
	ret = ku_msgrcv(msqid, &msg_buf, KUIPC_MAXMSG, type, IPC_NOWAIT);
	
	printf("received msg : %s, type[%ld] \n", msg_buf.text, msg_buf.type);
	printf("data length : %d\n", ret);
//	ku_msgclose(msqid);
}
