#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#define DEV_NAME "mod1_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1

#define SIMPLE_IOCTL_NUM 'z'
#define SIMPLE_INCREASE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)

void getInput(int dev1, int dev2);
void main(void){
	int dev1;
	int dev2;

	unsigned long value = 0;
	dev1 = open("/dev/mod1_dev", O_RDWR);
	dev2 = open("/dev/mod2_dev", O_RDWR);

	getInput(dev1, dev2);

	close(dev2);
	close(dev1);
}

void getInput(int dev1, int dev2){
	int n=-1;
	unsigned long value=0;
	
	while(n!=0){
		printf("select number\n");
		printf("0. Exit\n");
		printf("1. Increase Value\n");
		printf("2. Display Value on the Kernel\n");
		printf("\n");

		scanf("%d", &n);
		switch(n){
		case 0:
			return;
		case 1:
			ioctl(dev1,SIMPLE_INCREASE,&value);
			break;
		case 2:
			read(dev2,"",30);
			break;
		default:
			printf("Unvalid number\n");
			break;
		}
	}
}
