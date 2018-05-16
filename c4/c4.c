#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/unistd.h>
#include <linux/delay.h>

#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

#define PIR 17
#define LED1 4
#define LED2 27
#define LED3 5

struct my_data{
	int year;
	int month;
	int day;
};

struct task_struct *led2_task=NULL;
struct task_struct *led3_task=NULL;

static struct workqueue_struct *test_wq;

typedef struct {
	struct work_struct my_work;
	struct my_data data;
}my_work_t;

my_work_t *work;

static int irq_num;

void my_wq_func(struct work_struct *work){
	
	if(gpio_get_value(LED1)==0){
		gpio_set_value(LED1, 1);
	} else{
		gpio_set_value(LED1, 0);
	}
}

static irqreturn_t simple_pir_isr(int irq, void* data){
	int ret;
	printk("ISR start\n");
	
	if(test_wq){
		work = (my_work_t*)kmalloc(sizeof(my_work_t), GFP_KERNEL);
		if(work){
			INIT_WORK((struct work_struct*)work, my_wq_func);
			ret = queue_work(test_wq, (struct work_struct*)work);
		}
	}
	return IRQ_HANDLED;
}

int led2_thread(void *data){
	while(!kthread_should_stop()){
		if(gpio_get_value(LED2)==0){
			gpio_set_value(LED2, 1);
		} else{
			gpio_set_value(LED2, 0);
		}
		msleep(1000);
	}
	return 0;
}

int led3_thread(void *data){
	while(!kthread_should_stop()){
		if(gpio_get_value(LED3)==0){
			gpio_set_value(LED3, 1);
		} else{
			gpio_set_value(LED3, 0);
		}
		msleep(1500);
	}
	return 0;
}

static int __init kernthread_init(void){
	int ret;
	printk("Init Module\n");

	test_wq = create_workqueue("test_workqueue");

	gpio_request_one(PIR, GPIOF_IN, "PIR");
	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	gpio_request_one(LED2, GPIOF_OUT_INIT_LOW, "LED2");
	gpio_request_one(LED3, GPIOF_OUT_INIT_LOW, "LED3");

	
	irq_num = gpio_to_irq(PIR);
	ret = request_irq(irq_num, simple_pir_isr, IRQF_TRIGGER_FALLING, "pir_irq", NULL);
	if(ret){
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		free_irq(irq_num, NULL);
	}
	led2_task = kthread_create(led2_thread, NULL, "led2_thread");
	led3_task = kthread_create(led3_thread, NULL, "led3_thread");
	

	wake_up_process(led2_task);
	wake_up_process(led3_task);
	return 0;
}

static void __exit kernthread_exit(void){
	printk("Exit Module\n");

	free_irq(irq_num, NULL);
	gpio_free(PIR);
	gpio_free(LED1);
	gpio_free(LED2);
	gpio_free(LED3);
	if(led2_task){
		kthread_stop(led2_task);
		printk("test kernel thread STOP");
	}
	if(led3_task){
		kthread_stop(led3_task);
		printk("test kernel thread STOP");
	}
	flush_workqueue(test_wq);
	destroy_workqueue(test_wq);
}

module_init(kernthread_init);
module_exit(kernthread_exit);
