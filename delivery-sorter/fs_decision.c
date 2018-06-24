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

#include "fs_decision.h"


MODULE_LICENSE("GPL");

static void move_motor(int direction){
	int r,degree,delay;
	delay = 850;
	degree = 90;
	degree = degree*512/45;


	if(direction){
		for(r = degree-1; r>=0; --r){
			gpio_set_value(MOTOR_PIN1, steps[r%8][0]);
			gpio_set_value(MOTOR_PIN2, steps[r%8][1]);
			gpio_set_value(MOTOR_PIN3, steps[r%8][2]);
			gpio_set_value(MOTOR_PIN4, steps[r%8][3]);
			udelay(delay);
		}
	} else {
		for(r = 0; r<degree; --r){
			gpio_set_value(MOTOR_PIN1, steps[r%8][0]);
			gpio_set_value(MOTOR_PIN2, steps[r%8][1]);
			gpio_set_value(MOTOR_PIN3, steps[r%8][2]);
			gpio_set_value(MOTOR_PIN4, steps[r%8][3]);
			udelay(delay);
		}

	}
}


static int fs_open(struct inode *inode, struct file *file)
{
	printk("main_open\n");

	return 0;
}

static int fs_release(struct inode *inode, struct file *file)
{
	printk("main_release\n");
	return 0;
}

struct decision_buf{
	int d;
};

static int fs_write(struct file *file, const char *buf, size_t len, loff_t *lof){
	struct decision_buf db;
	if(copy_from_user(&db, buf, len)){
		return -EFAULT;
	}
	
	move_motor(db.d);

	printk("write %d", db.d);
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

	printk("init decision Module\n");
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &fs_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);

	if(ret < 0){
		printk("fail to insert drive\n");
		return -1;
	}

	gpio_request_one(MOTOR_PIN1, GPIOF_OUT_INIT_LOW, "motor2_pin1");
	gpio_request_one(MOTOR_PIN2, GPIOF_OUT_INIT_LOW, "motor2_pin2");
	gpio_request_one(MOTOR_PIN3, GPIOF_OUT_INIT_LOW, "motor2_pin3");
	gpio_request_one(MOTOR_PIN4, GPIOF_OUT_INIT_LOW, "motor2_pin4");

	return 0;
}

static void __exit fs_exit(void){
	printk("exit motor2  Module\n");

	gpio_set_value(MOTOR_PIN1, 0);
	gpio_set_value(MOTOR_PIN2, 0);
	gpio_set_value(MOTOR_PIN3, 0);
	gpio_set_value(MOTOR_PIN4, 0);

	gpio_free(MOTOR_PIN1);
	gpio_free(MOTOR_PIN2);
	gpio_free(MOTOR_PIN3);
	gpio_free(MOTOR_PIN4);
	
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(fs_init);
module_exit(fs_exit);
