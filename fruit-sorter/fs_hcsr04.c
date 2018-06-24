#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <asm/delay.h>
#include <linux/uaccess.h>

#include "fs_hcsr04.h"

long dist;

MODULE_LICENSE("GPL");

static int irq_num;
static struct timer_list my_timer;
static struct timeval after, before;

void my_timer_callback(unsigned long data){
    start_sonic();
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(20));
}

void start_sonic(void){
    gpio_set_value(TRIGGER, 1);
    udelay(10);
    gpio_set_value(TRIGGER, 0);
}

static irqreturn_t sonic_isr(int irq, void *dev_id){
    if(gpio_get_value(ECHO) == 1){
        do_gettimeofday(&before);
    }

    if(gpio_get_value(ECHO) == 0){
        do_gettimeofday(&after);
	dist=(after.tv_usec - before.tv_usec)/58;
    }

    return IRQ_HANDLED;
}

static int sonic_open(struct inode *inode, struct file* file){
	printk("hscr open\n");
	return 0;
}

static int sonic_release(struct inode *inode, struct file* file){
	printk("hscr close\n");
	return 0;
}

struct hcsr_buf{
	long dist;
};

static int sonic_read(struct file *file, char *buf, size_t len, loff_t *lof){
	printk("read: %d", dist);
	struct hcsr_buf h;
	h.dist = dist;
	if(copy_to_user(buf, &h, len)){
		return -EFAULT;
	}
	return 0;
}

struct file_operations sonic_fops = {
	.open = sonic_open,
	.release = sonic_release,
	.read = sonic_read,	
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_sensor_init(void){
    int ret;

    printk("Init Module\n");

    alloc_chrdev_region(&dev_num, 0, 1, DEVNAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &sonic_fops);
    cdev_add(cd_cdev, dev_num, 1);

    gpio_request_one(TRIGGER, GPIOF_OUT_INIT_LOW, "trigger");
    gpio_request_one(ECHO, GPIOF_IN, "echo");

    init_timer(&my_timer);

    setup_timer(&my_timer, my_timer_callback, 0);

    irq_num = gpio_to_irq(ECHO);
    ret = request_irq(irq_num, sonic_isr,IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "sonic_sensor", NULL);

    if(ret){
        printk( "Unable to request IRQ: %d\n", ret);
        free_irq(irq_num, NULL);
    } else {
        disable_irq(irq_num);
    }
    enable_irq(irq_num);

    mod_timer(&my_timer, jiffies + msecs_to_jiffies(20));

    return 0;
}

static void __exit simple_sensor_exit(void){
    printk("Exit Module\n");
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
    
    free_irq(irq_num, NULL);
    del_timer(&my_timer);
    gpio_free(TRIGGER);
    gpio_free(ECHO);
}

module_init(simple_sensor_init);
module_exit(simple_sensor_exit);
