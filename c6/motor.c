#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

#define STEPS 0

int steps[8][4]={
	{1,0,0,0},
	{1,1,0,0},
	{0,1,0,0},
	{0,1,1,0},
	{0,0,1,0},
	{0,0,1,1},
	{0,0,0,1},
	{1,0,0,1}
};
	

void moveDegree(int degree, int delay, int direction){
	int r;
	int tmp;
	tmp = degree*512/45;
	if(direction){
		for(r=tmp-1; r>=0; --r){
			gpio_set_value(PIN1,steps[r%8][0]);
			gpio_set_value(PIN2,steps[r%8][1]);
			gpio_set_value(PIN3,steps[r%8][2]);
			gpio_set_value(PIN4,steps[r%8][3]);
			udelay(delay);
		}
	
	} else{
		for(r=0; r<tmp; ++r){
			gpio_set_value(PIN1,steps[r%8][0]);
			gpio_set_value(PIN2,steps[r%8][1]);
			gpio_set_value(PIN3,steps[r%8][2]);
			gpio_set_value(PIN4,steps[r%8][3]);
			udelay(delay);
		}
	}
}

static int __init simple_motor_init(void){
	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

	moveDegree(360, 1200,0);
	mdelay(1000);
	
	moveDegree(180, 1200,1);
	mdelay(1000);
	moveDegree(90, 1200,0);
	mdelay(1000);
	moveDegree(360, 1200,1);
	
	return 0;
}

static void __exit simple_motor_exit(void){
	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);
}

module_init(simple_motor_init);
module_exit(simple_motor_exit);
