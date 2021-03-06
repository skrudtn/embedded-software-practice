#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define DIST		10
#define MAX_FREQ	10

void run_system(void);
void setup_right_color(void);
void setup_wrong_color(void);

struct hcsr_buf {
	unsigned long dist;
};

struct decision_buf {
	int d;
};

struct conveyor_buf{
	int run_motor;
};

struct tcs_measurement{
	uint32_t w_h;
	uint32_t r;
	uint32_t g;
	uint32_t b;
	uint32_t w_t;
};	

int hcsr04_fd, conveyor_fd, tcs_fd, decision_fd;

int right_color[5] = {0,};
int wrong_color[5] = {0,};
int r_c, w_c, add_cnt;

int color_arr[5][256] = {};

int isEntered;

int main(void){
	int i,j;
	char tmp;
	hcsr04_fd = open("/dev/sonic_dev", O_RDWR);
//	printf("hcsr04_fd %d\n", hcsr04_fd);

	tcs_fd = open("/dev/tcs_dev", O_RDWR);
//	printf("tcs_fd %d\n", tcs_fd);

	conveyor_fd = open("/dev/motor1_dev", O_RDWR);
//	printf("conveyor_fd %d\n", conveyor_fd);
	
	decision_fd = open("/dev/motor2_dev", O_RDWR);
//	printf("decision_fd %d\n", decision_fd);
	struct decision_buf db;
	db.d=1;
	write(decision_fd, &db, sizeof(struct decision_buf));
	
	printf("put first one\n");
	setup_right_color();
	add_cnt=0;
	for(i = 0; i<4; ++i){
		for(j=0; j<256; ++j){
			color_arr[i][j] = 0;
		}
	}
	printf("right_color : (%d,%d,%d,%d)\n", 
			right_color[0],
			right_color[1],
			right_color[2],
			right_color[3]);

	printf("put second one, press enter\n");
	scanf("%c", &tmp);

	setup_wrong_color();
	printf("wrong_color : (%d,%d,%d,%d)\n", 
			wrong_color[0],
			wrong_color[1],
			wrong_color[2],
			wrong_color[3]);

	run_system();

	close(decision_fd);
	close(conveyor_fd);
	close(tcs_fd);
	close(hcsr04_fd);

}

int get_max(int row){
	//printf("get_max :%d freq \n", row);
	int max_idx,i,max_val;
	max_idx=max_val= 0;
	
	for(i=0; i<256; ++i){
	//	printf("%d ", color_arr[row][i]);
		if(max_val<color_arr[row][i]){
			max_val=color_arr[row][i];
			max_idx=i;
		}
	}

	return max_idx;
}

void make_max_freq(int isRight){
	if(isRight){
		right_color[0] = get_max(0);
		right_color[1] = get_max(1);
		right_color[2] = get_max(2);
		right_color[3] = get_max(3);
	} else {
		wrong_color[0] = get_max(0);
		wrong_color[1] = get_max(1);
		wrong_color[2] = get_max(2);
		wrong_color[3] = get_max(3);
	}
}

void add_color_freq(long w, long r, long g, long b){
//	printf("cnt :%d, (%ld,%ld,%ld,%ld)\n", add_cnt,w,r,g,b);
	add_cnt++;
	color_arr[0][w] +=1;
	color_arr[1][r] +=1;
	color_arr[2][g] +=1;
	color_arr[3][b] +=1;
	
}

void setup_right_color(void){
	struct hcsr_buf h;
	struct conveyor_buf c;
	struct tcs_measurement m;
	int cnt=0;

	while(!r_c){
		read(hcsr04_fd,&h,sizeof(struct hcsr_buf));
		if(h.dist<DIST){
			if(cnt>MAX_FREQ) {
				make_max_freq(1);
				r_c=1;
			}


//			printf("dist %ld \n", h.dist);
			if(read(tcs_fd, &m, sizeof(struct tcs_measurement))==sizeof(struct tcs_measurement)){
//				printf("w:%d, r:%d, g:%d, b:%d\n", m.w_h, m.r,m.g,m.b);
				++cnt;
				add_color_freq(m.w_h, m.r, m.g, m.b);
			}

			c.run_motor = 0;
		} else{
			c.run_motor = 1;
		}

		write(conveyor_fd, &c, sizeof(struct conveyor_buf));
	}

}

void setup_wrong_color(void){
	struct hcsr_buf h;
	struct conveyor_buf c;
	struct tcs_measurement m;
	int cnt=0;

	while(!w_c){
		read(hcsr04_fd,&h,sizeof(struct hcsr_buf));
		if(h.dist<DIST){
			if(cnt>MAX_FREQ) {
				make_max_freq(0);
				w_c=1;
			}


//			printf("dist %ld \n", h.dist);
			if(read(tcs_fd, &m, sizeof(struct tcs_measurement))==sizeof(struct tcs_measurement)){
//				printf("w:%d, r:%d, g:%d, b:%d\n", m.w_h, m.r,m.g,m.b);
				++cnt;
				add_color_freq(m.w_h, m.r, m.g, m.b);
			}

			c.run_motor = 0;
		} else{
			c.run_motor = 1;
		}

		write(conveyor_fd, &c, sizeof(struct conveyor_buf));
	}

}

int is_correct_color(long w, long r, long g, long b){
	return w>right_color[0]-5 && w<right_color[0]+5 && 
		r>right_color[1]-5 && r<right_color[1]+5 && 
		g>right_color[2]-5 && g<right_color[2]+5 && 
		b>right_color[3]-5 && b<right_color[3]+5 ;
}

void run_system(void){
	struct hcsr_buf h;
	struct conveyor_buf c;
	struct decision_buf db;
	struct tcs_measurement m;

	printf("system start\n");

	for(;;){
		read(hcsr04_fd,&h,sizeof(struct hcsr_buf));
		if(h.dist<DIST){
			printf("dist %ld \n", h.dist);
			c.run_motor = 0;
			write(conveyor_fd, &c, sizeof(struct conveyor_buf));
			
			read(tcs_fd, &m, sizeof(struct tcs_measurement));
	//		printf("r:%d, g:%d, b:%d\n", m.r,m.g,m.b);

			if(is_correct_color(m.w_t,m.r,m.g,m.b)){
				printf("Correct color!\n");
				db.d = 1;
			} else{
				db.d = 0;

			}
			printf("decision %d\n", db.d);
			//write(decision_fd, &db, sizeof(struct decision_buf));
			
			c.run_motor = 1;
			write(conveyor_fd, &c, sizeof(struct conveyor_buf));
			
		//	sleep(1);
		}	
	}

}
