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
			if(isExist){
				ret = -1;
			} else {
				ret = ioctl(dev, KU_IOCTL_CREATE_QUEUE, &key);
			}
			break;
		default:
			if(isExist){
				ret = isExist;
			} else {
				ret = ioctl(dev, KU_IOCTL_CREATE_QUEUE, &key);
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
	struct ipcbuf ipc_buf;

	dev = getDev();
	ipc_buf.msqid = msqid;
	ipc_buf.msgp = msgp;
	ipc_buf.msgsz = msgsz;
	ipc_buf.msgflg = msgflg;
	
	
	isFullQueue = ioctl(dev, KU_IOCTL_IS_FULL_QUEUE, &msqid);
	if(isFullQueue){
		if(msgflg & IPC_NOWAIT){
			return ret=-1;
		} else {
			while(ioctl(dev, KU_IOCTL_IS_FULL_QUEUE, &msqid) || ioctl(dev, KU_IOCTL_IS_NO_QUEUE));
		}

	}
	remainedByte = ioctl(dev, KU_IOCTL_SND, &ipc_buf);
	return ret;
}

int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg){
	int dev;
	int ret=0;
	int remainedByte;
	int isEmptyQueue;
	int rcved_msg_size;
	struct msgbuf *tmp;
	struct ipcbuf ipc_buf;
	char cutchar[msgsz];

	dev = getDev();
	ipc_buf.msqid = msqid;
	ipc_buf.msgp = msgp;
	ipc_buf.msgsz = msgsz;
	ipc_buf.msgtyp = msgtyp;
	ipc_buf.msgflg = msgflg;
	
	isEmptyQueue = ioctl(dev, KU_IOCTL_IS_EMPTY_QUEUE, &msqid);
	
	if(isEmptyQueue){
		if(msgflg & IPC_NOWAIT){
			return ret=-1;
		} else {
			while(ioctl(dev, KU_IOCTL_IS_EMPTY_QUEUE, &msqid) || ioctl(dev, KU_IOCTL_IS_NO_QUEUE));
		}

	}

	
	remainedByte = ioctl(dev, KU_IOCTL_RCV, &ipc_buf);

	tmp = (struct msgbuf*)ipc_buf.msgp;
	ret = strlen(tmp->text);

	rcved_msg_size = sizeof(*tmp);
	
	if(msgsz < rcved_msg_size){
		if(!(msgflg & MSG_NOERROR)){
			ret = -1;
		}
	}

	close(dev);
	return ret;
}

int getDev(){
	return open("/dev/"DEV_NAME, O_RDWR);
}
