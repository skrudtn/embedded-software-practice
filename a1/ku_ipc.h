#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4
#define IOCTL_NUM5 IOCTL_START_NUM+5
#define IOCTL_NUM6 IOCTL_START_NUM+6
#define IOCTL_NUM7 IOCTL_START_NUM+7
#define IOCTL_NUM8 IOCTL_START_NUM+8
#define IOCTL_NUM9 IOCTL_START_NUM+9

#define SIMPLE_IOCTL_NUM 'z'
#define KU_IOCTL_SND 		_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_IOCTL_RCV 		_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define KU_IOCTL_MSGGET 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define KU_IOCTL_MSGCLOSE 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long *)
#define KU_IOCTL_IS_EXIST_KEY	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM5, unsigned long *)
#define KU_IOCTL_CREATE_QUEUE 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM6, unsigned long *)
#define KU_IOCTL_IS_FULL_QUEUE 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM7, unsigned long *)
#define KU_IOCTL_IS_NO_QUEUE 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM8, unsigned long *)
#define KU_IOCTL_IS_EMPTY_QUEUE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM9, unsigned long *)

#define IPC_FLAG	1
#define IPC_CREAT 	IPC_FLAG<<1
#define IPC_EXCL 	IPC_FLAG<<2
#define IPC_NOWAIT 	IPC_FLAG<<3
#define MSG_NOERROR 	IPC_FLAG<<4


#define KUIPC_MAXMSG 	10	/* maximum number of massages per queue */
#define KUIPC_MAXVOL 	1000	/* maximum data volume allowed for each queue */

#define DEV_NAME "ku_ipc_dev"

struct msgbuf{
	long type;		/* Type of message */
	char text[100];		/* Message text*/
};

struct ipcbuf{
	int msqid;
	void *msgp;
	int msgsz;
	long msgtyp;
	int msgflg;
};
