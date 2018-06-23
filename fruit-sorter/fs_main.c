#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/cdev.h>

#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/timer.h>

#include "config.h"
#include "common.h"
#include "motor.h"

MODULE_LICENSE("GPL");

extern int dist;
struct task_struct *conveyor_task = NULL;
static struct timer_list conveyor_timer;
extern long get_distance(void);

static void conveyor_timer_func(unsigned long data){
	conveyor_timer.expires = jiffies + (0.1*HZ);
	run_conveyor_motor();
	add_timer(&conveyor_timer);
}

static int run_conveyor_motor(void){
	int r,degree,i;
	r=0;
	degree = 15;
	degree = degree*512/45;
	printk("run conveyor\n");
	
	for(i=0; i<2; ++i){
		printk("motor cycle %d\n", i);	
		printk("dist :%ld", dist);
		printk("get_distance :%ld", get_distance());
		while(r<degree){
			if(dist<5) break;
			gpio_set_value(MOTOR_PIN1, steps[r%8][0]);
			gpio_set_value(MOTOR_PIN2, steps[r%8][1]);
			gpio_set_value(MOTOR_PIN3, steps[r%8][2]);
			gpio_set_value(MOTOR_PIN4, steps[r%8][3]);
			udelay(1200);
			r++;
		}
		r=0;
	}
}

static int motor_init(void){
	gpio_request_one(MOTOR_PIN1, GPIOF_OUT_INIT_LOW, "motor_pin1");
	gpio_request_one(MOTOR_PIN2, GPIOF_OUT_INIT_LOW, "motor_pin2");
	gpio_request_one(MOTOR_PIN3, GPIOF_OUT_INIT_LOW, "motor_pin3");
	gpio_request_one(MOTOR_PIN4, GPIOF_OUT_INIT_LOW, "motor_pin4");
	
//	run_conveyor_motor();
	
	init_timer(&conveyor_timer);
	conveyor_timer.function = conveyor_timer_func;
	conveyor_timer.data = 1L;
	conveyor_timer.expires = jiffies+(0.1*HZ);
	
	add_timer(&conveyor_timer);
	
/*
	conveyor_task = kthread_create(run_conveyor_motor, NULL, "conveyor_thread");
	if(IS_ERR(conveyor_task)){
		conveyor_task = NULL;
		printk("Conveyor_motor thread ERROR\n");
	}
	
	wake_up_process(conveyor_task);
*/
}

static void motor_exit(void){
	gpio_set_value(MOTOR_PIN1, 0);
	gpio_set_value(MOTOR_PIN2, 0);
	gpio_set_value(MOTOR_PIN3, 0);
	gpio_set_value(MOTOR_PIN4, 0);
	
	gpio_free(MOTOR_PIN1);
	gpio_free(MOTOR_PIN2);
	gpio_free(MOTOR_PIN3);
	gpio_free(MOTOR_PIN4);

	if(conveyor_task){
		kthread_stop(conveyor_task);
		printk("Conveyor motor thread STOP\n");
	}
}


static int fs_open(struct inode *inode, struct file *file)
{
	printk("open\n");
	return 0;
}

static int fs_release(struct inode *inode, struct file *file)
{
	printk("release\n");
	return 0;
}

static long fs_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
}

static struct file_operations fs_fops = {
	.open = fs_open,
	.release = fs_release,
	.unlocked_ioctl = fs_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init fs_init(void){
	printk("init Module\n");
	alloc_chrdev_region(&dev_num, 0, 1, "fruit_sorter");
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &fs_fops);
	cdev_add(cd_cdev, dev_num, 1);
//	tcs3200_init();
	hcsr04_init();
	motor_init();
}

static void __exit fs_exit(void){
	printk("exit Module\n");
	hcsr04_exit();
	motor_exit();
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(fs_init);
module_exit(fs_exit);
