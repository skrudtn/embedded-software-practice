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

#include "config.h"
#include "common.h"

int isEntered;
long dist;
EXPORT_SYMBOL(dist);
static int irq_num;
static struct timer_list my_timer;
static struct timeval after, before;

static void my_timer_func(unsigned long data){
	my_timer.expires = jiffies + (1*HZ);
	hcsr04_setup();
	add_timer(&my_timer);
}

void hcsr04_setup(void){
	gpio_set_value(HCSR04_TRIGGER, 1);
	udelay(10);
	gpio_set_value(HCSR04_TRIGGER, 0);
}

long get_distance(void){
	return dist;
}
EXPORT_SYMBOL(get_distance);

static irqreturn_t hcsr04_isr(int irq, void *dev_id){
	long distance;
	if(gpio_get_value(HCSR04_ECHO) == 1){
		do_gettimeofday(&before);
	}

	if(gpio_get_value(HCSR04_ECHO) == 0){
		do_gettimeofday(&after);
		distance = (after.tv_usec - before.tv_usec)/58;
		dist= distance;
		printk("distance %ld cm\n\n", distance);
		if(distance < 3){
			isEntered = 1;
			printk("hcsr04 isEnterd\n");
		}
		else
			isEntered = 0;
	}

	return IRQ_HANDLED;
}

void hcsr04_init(void){
	int ret;

	printk("Init Module\n");
	isEntered = 0;

	gpio_request_one(HCSR04_TRIGGER, GPIOF_OUT_INIT_LOW, "trigger");
	gpio_request_one(HCSR04_ECHO, GPIOF_IN, "echo");

	init_timer(&my_timer);
	my_timer.function = my_timer_func;
	my_timer.expires = jiffies + (1*HZ);

	irq_num = gpio_to_irq(HCSR04_ECHO);
	ret = request_irq(irq_num, hcsr04_isr,IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "sonic_sensor", NULL);

	if(ret){
		printk("Unable to request IRQ: %d\n", ret);
		free_irq(irq_num, NULL);
	} else {
		disable_irq(irq_num);
	}

	enable_irq(irq_num);
	add_timer(&my_timer);
}

void hcsr04_exit(void){
	free_irq(irq_num, NULL);
	del_timer(&my_timer);
	gpio_free(HCSR04_TRIGGER);
	gpio_free(HCSR04_ECHO);
}
