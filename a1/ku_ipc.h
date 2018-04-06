#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4
#define IOCTL_NUM5 IOCTL_START_NUM+5
#define IOCTL_NUM6 IOCTL_START_NUM+6

#define SIMPLE_IOCTL_NUM 'z'
#define KU_IOCTL_SND 		_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_IOCTL_RCV 		_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define KU_IOCTL_MSGGET 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define KU_IOCTL_MSGCLOSE 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long *)
#define KU_IOCTL_IS_EXIST_KEY	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM5, unsigned long *)
#define KU_IOCTL_CREATE_QUEUE 	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM6, unsigned long *)


#define IPC_CREAT 	0x1000
#define IPC_EXCL 	0x1001
#define IPC_NOWAIT 	0x1002
#define IPC_NOERROR 	0x1003

#define KU_IPC_EXCL	0x2000

#define KUIPC_MAXMSG 	128	/* maximum number of massages pre queue */
#define KUIPC_MAXVOL 	2048	/* maximum data volume allowed for each queue */

#define DEV_NAME "ku_ipc_dev"

struct msgbuf{
	long type;		/* Type of message */
	char text[KUIPC_MAXMSG];/* Message text*/
};

struct ipcbuf{
	int msqid;
	void *msgp;
	int msgsz;
	long msgtyp;
	int msgflg;
};
/*
int ku_msgget(int key, int msgflg);
int ku_msgclose(int msqid);
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg);
int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg);
*/
