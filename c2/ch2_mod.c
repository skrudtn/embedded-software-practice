#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

#define LED1 4
#define LED2 17
#define LED3 27

static struct timer_list my_timer;

static void my_timer_func(unsigned long data){
	
	int lednum = data%3;
	switch(lednum){
		case 0:
			gpio_set_value(LED1, 1);
			gpio_set_value(LED2, 0);
			gpio_set_value(LED3, 0);
			break;
		case 1:
			gpio_set_value(LED1, 0);
			gpio_set_value(LED2, 1);
			gpio_set_value(LED3, 0);
			break;
		case 2:
			gpio_set_value(LED1, 0);
			gpio_set_value(LED2, 0);
			gpio_set_value(LED3, 1);
			break;
		default:
			break;
	}

	my_timer.data = data+1;
	my_timer.expires = jiffies + (1*HZ);

	add_timer(&my_timer);
}

static int __init simple_led_init(void){
	init_timer(&my_timer);

	my_timer.function = my_timer_func;
	my_timer.data = 0L;
	my_timer.expires = jiffies+(1*HZ);
	add_timer(&my_timer);

	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	gpio_request_one(LED2, GPIOF_OUT_INIT_LOW, "LED2");
	gpio_request_one(LED3, GPIOF_OUT_INIT_LOW, "LED3");

	return 0;
}

static void __exit simple_led_exit(void){
	gpio_set_value(LED1, 0);
	gpio_set_value(LED2, 0);
	gpio_set_value(LED3, 0);

	gpio_free(LED1);
	gpio_free(LED2);
	gpio_free(LED3);

	del_timer(&my_timer);

}

module_init(simple_led_init);
module_exit(simple_led_exit);


