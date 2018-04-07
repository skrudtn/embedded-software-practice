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
	strcpy(msg, "");
	if(argc == 1){
		return;
	} else{
		for(i=2; i<argc; i++){
			strncat(msg,argv[i],strlen(argv[i]));
			strncat(msg," ",1);
		}
	}
	
	key = 1;
	//msqid = ku_msgget(atoi(argv[1]), IPC_CREAT);
	//msqid = ku_msgget(key, IPC_EXCL);
	msqid = ku_msgget(key, IPC_CREAT);
	if(msqid == -1){
		printf("already exist key : %d", key);
		return;
	}
	printf("msqid : %d\n", msqid);
	strncpy(msg_buf.text, msg, sizeof(msg));
//	msg_buf.type = TYPE;
	msg_buf.type = atoi(argv[1]);

	if(ku_msgsnd(msqid, &msg_buf, sizeof(msg_buf), IPC_NOWAIT) == 0){
		printf("success sned msg : %s\n", msg);
	} else {
		printf("failed to send\n");
	}
	
	/*
	if(ku_msgclose(msqid) == -1){
		printf("failed to msgclose, msqid[%d] is not exist\n", msqid);
	} else {
		printf("success to msgclose\n");
	}
	*/
}
