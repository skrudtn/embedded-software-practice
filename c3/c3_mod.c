#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

#define SENSOR1 17
#define LED1	27

#define DEV_NAME "c3_dev"

static int irq_num;
static struct timer_list my_timer;

static void my_timer_func(unsigned long data){
	gpio_set_value(LED1, 0);
	
}

static int simple_sensor_open(struct inode *inode, struct file* file){
	printk("simple_sensor open\n");
	enable_irq(irq_num);
	return 0;
}

static int simple_sensor_release(struct inode *inode, struct file* file){
	printk("simple_sensor close\n");
	disable_irq(irq_num);
	return 0;
}

struct file_operations simple_sensor_fops =
{
	.open = simple_sensor_open,
	.release = simple_sensor_release,
};

static irqreturn_t simple_sensor_isr(int irq, void* dev_id){
	printk("detect \n");
	gpio_set_value(LED1, 1);
	my_timer.expires = jiffies + (2*HZ);
	add_timer(&my_timer);

	return IRQ_HANDLED;
}

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_sensor_init(void){
	int ret;

	printk("Init Module\n");
	/* allocate character device*/
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &simple_sensor_fops);
	cdev_add(cd_cdev, dev_num, 1);
	
	/* init timer */
	init_timer(&my_timer);
	my_timer.function = my_timer_func;
	my_timer.data = 0L;
	my_timer.expires = jiffies + (2*HZ);

	/* requset GPIO and interrupt handler */
	gpio_request_one(SENSOR1, GPIOF_IN, "sensor1");
	gpio_request_one(LED1, GPIOF_INIT_LOW, "LED1");
	
	gpio_set_value(LED1, 0);

	irq_num = gpio_to_irq(SENSOR1);
	printk("%d", irq_num);
	ret = request_irq(irq_num, simple_sensor_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if(ret){
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		free_irq(irq_num, NULL);
	} else {
		printk("irs success");
	}
	return 0;
}

static void __exit simple_sensor_exit(void){
	printk("Exit Module \n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);

	del_timer(&my_timer);

	free_irq(irq_num, NULL);
	gpio_set_value(LED1, 0);

	gpio_free(LED1);
	gpio_free(SENSOR1);
}

module_init(simple_sensor_init);
module_exit(simple_sensor_exit);

