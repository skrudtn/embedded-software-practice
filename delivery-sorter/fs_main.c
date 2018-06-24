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
#include <linux/ktime.h>
#include <linux/timer.h>

#include "config.h"
#include "fs_main.h"

#define DEV_NAME "motor1_dev"

MODULE_LICENSE("GPL");

static int run_motor;
static struct hrtimer timer;

static void run_conveyor_motor(void){
	int r,degree;
	r=0;
	degree = 1;
	degree = degree*512/45;

/*
	while(r<degree){
		if(!run_motor){
			break;
		}

		gpio_set_value(MOTOR_PIN1, steps[r%8][0]);
		gpio_set_value(MOTOR_PIN2, steps[r%8][1]);
		gpio_set_value(MOTOR_PIN3, steps[r%8][2]);
		gpio_set_value(MOTOR_PIN4, steps[r%8][3]);
		udelay(850);
		r++;
	}
*/
	// reverse turn
	r = degree-1;

	while(r>=0){
		if(!run_motor){
			break;
		}

		gpio_set_value(MOTOR_PIN1, steps[r%8][0]);
		gpio_set_value(MOTOR_PIN2, steps[r%8][1]);
		gpio_set_value(MOTOR_PIN3, steps[r%8][2]);
		gpio_set_value(MOTOR_PIN4, steps[r%8][3]);
		udelay(850);
		r--;
	}
}


static enum hrtimer_restart timer_func(struct hrtimer *hr_timer){
	run_conveyor_motor();
	hrtimer_forward_now(&timer, ns_to_ktime(SLEEP_SPEED));
	return HRTIMER_RESTART;
}


static int fs_open(struct inode *inode, struct file *file)
{
	printk("main_open\n");
	hrtimer_start(&timer, ns_to_ktime(SLEEP_SPEED), HRTIMER_MODE_REL);

	return 0;
}

static int fs_release(struct inode *inode, struct file *file)
{
	printk("main_release\n");
	hrtimer_cancel(&timer);
	return 0;
}

struct conveyor_buf{
	int run_motor;
};

static int fs_write(struct file *file, const char *buf, size_t len, loff_t *lof){
	struct conveyor_buf cb;
	if(copy_from_user(&cb, buf, len)){
		return -EFAULT;
	}
	
	run_motor = cb.run_motor;
	printk("write %d",run_motor);
	return 0;
}

static struct file_operations fs_fops = {
	.open = fs_open,
	.release = fs_release,
	.write = fs_write,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init fs_init(void){
	int ret;

	printk("init fs_main Module\n");
	run_motor = 1;
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &fs_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);

	if(ret < 0){
		printk("fail to insert drive\n");
		return -1;
	}

	gpio_request_one(MOTOR_PIN1, GPIOF_OUT_INIT_LOW, "motor_pin1");
	gpio_request_one(MOTOR_PIN2, GPIOF_OUT_INIT_LOW, "motor_pin2");
	gpio_request_one(MOTOR_PIN3, GPIOF_OUT_INIT_LOW, "motor_pin3");
	gpio_request_one(MOTOR_PIN4, GPIOF_OUT_INIT_LOW, "motor_pin4");
	
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer.function = &timer_func;

	//hrtimer_start(&hrt, ns_to_ktime(SLEEP_SPEED), HRTIMER_MODE_REL);

	return 0;
}

static void __exit fs_exit(void){
	printk("exit fs_main Module\n");

	gpio_set_value(MOTOR_PIN1, 0);
	gpio_set_value(MOTOR_PIN2, 0);
	gpio_set_value(MOTOR_PIN3, 0);
	gpio_set_value(MOTOR_PIN4, 0);

	gpio_free(MOTOR_PIN1);
	gpio_free(MOTOR_PIN2);
	gpio_free(MOTOR_PIN3);
	gpio_free(MOTOR_PIN4);
	//hrtimer_cancel(&hrt);
	
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(fs_init);
module_exit(fs_exit);
