#include <stdio.h>
#include <fcntl.h>

#define TCS_DEV "/dev/tcs_dev"
#define DECIDE_DEV "/dev/decision_dev"

void run_system(void);

struct hcsr_buf {
	long dist;
};
struct conveyor_buf{
	int run_motor;
};

int hcsr04_fd, conveyor_fd, tcs_fd, decision_fd;

int main(void){

	
	hcsr04_fd = open("/dev/sonic_dev", O_RDWR);
	printf("hcsr04_fd %d\n", hcsr04_fd);

	//tcs_fd = open("/dev/tcs_dev", O_RDWR);
	
	conveyor_fd = open("/dev/motor1_dev", O_RDWR);
	printf("conveyor_fd %d\n", conveyor_fd);
	
/*
	if(decision_fd = open(DECIDE_DEV, O_RDONLY)<0){
		perror("decision open failed");
		return -1;
	}
*/

	run_system();

	close(hcsr04_fd);
	close(conveyor_fd);
}


void run_system(void){

	struct hcsr_buf h;
	struct conveyor_buf c;

	printf("system start\n");

	for(;;){
		read(hcsr04_fd,&h,sizeof(struct hcsr_buf));
//		printf("dist %ld \n", h.dist);
		if(h.dist<10){
			printf("dist %ld \n", h.dist);
			c.run_motor = 0;
		} else{
			c.run_motor = 1;
		}
		write(conveyor_fd, &c, sizeof(struct conveyor_buf));
	}

}
