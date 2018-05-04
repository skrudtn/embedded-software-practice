#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ku_pir.h"

int ku_pir_open(){
	int dev;
	int fd;

	dev = get_dev();
	if(dev<=0) 
		fd = -1;
	else 
		fd = ioctl(dev, KU_IOCTL_GET_FD, NULL);
	
	close(dev);
	return fd;
}

int ku_pir_close(int fd){
	int dev;
	int ret;

	dev = get_dev();

	ret = ioctl(dev, KU_IOCTL_CLOSE, &fd);
	
	close(dev);

	return ret;
}

void ku_pir_read(int fd, struct ku_pir_data *data){
	int dev;
	struct comb_data cd;
	
	dev = get_dev();
	cd.data = data;
	cd.fd = fd;

	ioctl(dev, KU_IOCTL_READ, &cd);
	close(dev);
}

void ku_pir_flush(int fd){
	int dev;
	
	dev = get_dev();
	printf("flush %d\n", fd);
	ioctl(dev, KU_IOCTL_FLUSH, &fd);

	close(dev);
}

int ku_pir_insertData(int fd, long unsigned int ts, char rf_flag){
	int ret;
	int dev;
	struct comb_data cd;
	
	cd.data = (struct ku_pir_data*)malloc(sizeof(struct ku_pir_data));
	dev = get_dev();
	ret = 0;
	cd.data->timestamp = ts;
	cd.data->rf_flag = rf_flag;
	cd.fd=fd;

	printf("fd: %d, ts: %ld, flag : %c\n", cd.fd, cd.data->timestamp, cd.data->rf_flag);
	ret = ioctl(dev, KU_IOCTL_INSERT, &cd); 

	close(dev);
	return ret;
}

int get_dev(){
	return open("/dev/"DEV_NAME, O_RDWR);
}
