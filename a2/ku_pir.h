#define KUPIR_MAX_MSG 10
#define KUPIR_SENSOR 17
#define DEV_NAME "ku_pir_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4
#define IOCTL_NUM5 IOCTL_START_NUM+5

#define SIMPLE_IOCTL_NUM 'z'
#define	KU_IOCTL_INSERT	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define	KU_IOCTL_READ	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define	KU_IOCTL_CLOSE	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define	KU_IOCTL_GET_FD	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long *)
#define	KU_IOCTL_FLUSH	_IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM5, unsigned long *)


struct ku_pir_data {
	long unsigned int timestamp;
	char rf_flag;
};

struct comb_data{
	struct ku_pir_data *data;
	int fd;
};
