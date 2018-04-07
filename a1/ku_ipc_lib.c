#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include "ku_ipc.h"

int ku_msgget(int key, int msgflg){
	int dev;
	int ret;
	int isExist;

	dev = getDev();
	isExist = ioctl(dev, KU_IOCTL_IS_EXIST_KEY, &key);
	switch(msgflg){
		case IPC_EXCL:
			if(isExist == -1){
				ret = ioctl(dev, KU_IOCTL_CREATE_QUEUE, &key);
			} else {
				ret = -1;
			}
			break;
		default:
			if(isExist == -1){
				ret = ioctl(dev, KU_IOCTL_CREATE_QUEUE, &key);
			} else {
				ret = isExist;
			}
			break;
	}
	close(dev);

	return ret;
}

int ku_msgclose(int msqid){
	int dev;
	int ret;

	dev = getDev();
	ret = ioctl(dev, KU_IOCTL_MSGCLOSE, &msqid);

	close(dev);

	return ret;
}

int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg){
	int dev;
	int ret=0;
	int remainedByte;
	int isFullQueue;
	struct msgbuf *msg_buf;
	struct ipcbuf ipc_buf;

	dev = getDev();
	msg_buf = (struct msgbuf*)msgp;
	ipc_buf.msqid = msqid;
	ipc_buf.msgp = msgp;
	ipc_buf.msgsz = msgsz;
	ipc_buf.msgflg = msgflg;
	
	
	isFullQueue = ioctl(dev, KU_IOCTL_IS_FULL_QUEUE, &msqid);
	printf("isFullQueue %d\n", isFullQueue);
	if(isFullQueue){
		if(msgflg & IPC_NOWAIT){
			return ret=-1;
		} else {
			while(ioctl(dev, KU_IOCTL_IS_FULL_QUEUE, &msqid) || ioctl(dev, KU_IOCTL_IS_NO_QUEUE));
		}

	}
	remainedByte = ioctl(dev, KU_IOCTL_SND, &ipc_buf);
	printf("remained Byte : %d\n", remainedByte);
	return ret;
}

int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg){
	int dev;
	int ret=0;
	int remainedByte;
	int isEmptyQueue;
	struct ipcbuf ipc_buf;

	dev = getDev();
	ipc_buf.msqid = msqid;
	ipc_buf.msgp = msgp;
	ipc_buf.msgsz = msgsz;
	ipc_buf.msgtyp = msgtyp;
	ipc_buf.msgflg = msgflg;
	
	printf("msqid %d\n", msqid);
	printf("dev %d\n", dev);

	isEmptyQueue = ioctl(dev, KU_IOCTL_IS_EMPTY_QUEUE, &msqid);
	printf("isEmptyQueue %d\n", isEmptyQueue);
	
	if(isEmptyQueue){
		if(msgflg & IPC_NOWAIT){
			return ret=-1;
		} else {
			while(ioctl(dev, KU_IOCTL_IS_EMPTY_QUEUE, &msqid) || ioctl(dev, KU_IOCTL_IS_NO_QUEUE));
		}

	}
	/* to do msg_noerror */
	
	remainedByte = ioctl(dev, KU_IOCTL_RCV, &ipc_buf);
	printf("ku_msgrcv: %s\n", ((struct msgbuf*)ipc_buf.msgp)->text);
	ret = strlen(((struct msgbuf*)ipc_buf.msgp)->text);
	close(dev);
	return ret;
}

int getDev(){
	return open("/dev/"DEV_NAME, O_RDWR);
}
