#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/rculist.h>


#include "ku_pir.h"

MODULE_LICENSE("GPL");

struct q* get_queue(int fd);
void insert_all(long unsigned int ts, char rf_flag);
int is_full_queue(int fd);
int get_num_of_data(int fd);
void poll_q(int fd);
int flush_queue(int fd);
int rm_queue(int fd);
int is_no_queue(int fd);

static int irq_num;
static int kern_fd;
spinlock_t my_lock;
wait_queue_head_t my_wq;

struct q{
	struct list_head list;
	struct ku_pir_data kern_data;
};

struct queues{
	struct list_head list;
	struct q kern_q;
	int fd;
};

struct queues kern_queues;

struct q* get_queue(int fd){
	struct queues *tmp_queues = 0;
	struct q *ret_q = 0;

	/* locking */
	rcu_read_lock();
	list_for_each_entry_rcu(tmp_queues, &kern_queues.list, list){
		if(tmp_queues->fd == fd)
			ret_q = &tmp_queues->kern_q;
	}
	rcu_read_unlock();
	/* unlocking */

	return ret_q;
}

int mk_queue(void){
	struct queues *new_queues =0;
	new_queues = (struct queues*)kmalloc(sizeof(struct queues), GFP_KERNEL);
	new_queues->fd = kern_fd++;
	
	INIT_LIST_HEAD(&new_queues->kern_q.list);

	/* locking */
	spin_lock(&my_lock); /* spin lock for multi reader process */
	list_add_rcu(&new_queues->list, &kern_queues.list);
	spin_unlock(&my_lock);
	/* unlocking */
	synchronize_rcu();	

	return new_queues->fd;
}

int is_full_queue(int fd){
	int ret = 0;

	if(get_num_of_data(fd) >= KUPIR_MAX_MSG)
		ret = 1;
	
	return ret;
}

int get_num_of_data(int fd){
	int ret = 0;
	struct q *tmp = 0;
	struct q *target_q =0;
	
	if(is_no_queue(fd)){
		return -1;
	}
	target_q = get_queue(fd);

	/* locking */
	spin_lock(&my_lock);
	list_for_each_entry(tmp, &target_q->list, list){
		ret += 1;
	}
	spin_unlock(&my_lock);
	/* unlocking */
	return ret;
}

void poll_q(int fd){
	struct q *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	struct q *target_q =0;
	
	target_q = get_queue(fd);

	/*locking */
	spin_lock(&my_lock);
	list_for_each_safe(pos, q, &target_q->list){
		tmp = list_entry(pos, struct q, list);
		list_del(pos); 
		/* q is refered in get_num_of_data func */
		
		kfree(tmp);
		break;
	}
	spin_unlock(&my_lock);
	/* unlocking */
}

int flush_queue(int fd){
	int ret;
	struct q *target_q = 0;
	struct q *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	
	ret = -1;
	if(is_no_queue(fd)){
		return ret;
	}
	target_q = get_queue(fd);
	/* locking */
	spin_lock(&my_lock);
	list_for_each_safe(pos, q, &target_q->list){
		tmp = list_entry(pos, struct q, list);
		list_del(pos);
		kfree(tmp);
		ret = 0;
	}
	spin_unlock(&my_lock);
	/* unlocking */
	return ret;
}

int rm_queue(int fd){
	struct queues *tmp = 0;
	struct list_head *pos;
	int ret;

	ret = -1;
	if(is_no_queue(fd)){
		printk("rm:%d, is no queue", fd);
		return ret;
	}
	flush_queue(fd);
	/* locking */
	spin_lock(&my_lock);
	list_for_each_entry_rcu(tmp, &kern_queues.list, list){
		if(tmp->fd == fd){
			list_del_rcu(&tmp->list);
			kfree(tmp);
			break;
		}
	}
	synchronize_rcu();
	if(is_no_queue(fd))
		ret = 0;
	spin_unlock(&my_lock);
	/* unlocking */
	return ret;
}

int is_no_queue(int fd){
	int ret;
	struct q *target_q = 0;
	target_q = get_queue(fd);
	ret = 0;
	if(target_q == NULL){
		ret = 1;
	}
	
	return ret;
}

int insert_one(struct comb_data *cd){
	int ret;
	struct q *tmp_q = 0;
	struct q *written_q = 0;
	
	if(is_no_queue(cd->fd)){
		printk("insert one %d, is no queue",cd->fd);
		return -1;
	}

	written_q = get_queue(cd->fd);

	tmp_q = (struct q*)kmalloc(sizeof(struct q), GFP_KERNEL);
	
	ret = copy_from_user(&(tmp_q->kern_data), cd->data, sizeof(struct ku_pir_data));
	
	printk("insert one %ld, %c", tmp_q->kern_data.timestamp, tmp_q->kern_data.rf_flag);
	/* check full queue*/
	if(is_full_queue(cd->fd)){
		poll_q(cd->fd);
	}
	
	/* locking */
	spin_lock(&my_lock);
	list_add_tail(&tmp_q->list, &written_q->list);
	spin_unlock(&my_lock);
	/* unlocking */

	wake_up_interruptible(&my_wq);
	return ret;
}

void insert_all(long unsigned int ts, char rf_flag){
	struct queues *tmp_queues = 0;
	struct q *new_q = 0;

	/* locking */
	rcu_read_lock();
	list_for_each_entry_rcu(tmp_queues, &kern_queues.list, list){
		new_q = (struct q*)kmalloc(sizeof(struct q), GFP_KERNEL);
		new_q->kern_data.timestamp = ts;
		new_q->kern_data.rf_flag = rf_flag;
		/* check full */
		
		if(is_full_queue(tmp_queues->fd)){
			poll_q(tmp_queues->fd);
		}


		spin_lock(&my_lock);
		list_add_rcu(&new_q->list, &tmp_queues->kern_q.list);
	//	printk("i, fd:%d, ts:%ld ", tmp_queues->fd, ts);
		spin_unlock(&my_lock);
	}
	rcu_read_unlock();

	wake_up_interruptible(&my_wq);

	/* unlocking */
}

static int ku_pir_read(struct comb_data *cd){
	struct q *read_q = 0;
	struct q *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	int ret;
	
	
	printk("fd:%d waiting... cnt:%d",cd->fd,get_num_of_data(cd->fd));
	wait_event_interruptible(my_wq, (get_num_of_data(cd->fd)>0) || (get_num_of_data(cd->fd)==-1));
	printk("fd:%d wake up!!! cnt:%d",cd->fd,get_num_of_data(cd->fd));
	
	if(is_no_queue(cd->fd)){
		printk("read fd :%d, is no queue",cd->fd);
		return -1;
	}
	read_q = get_queue(cd->fd);
	/* locking */
	spin_lock(&my_lock);
	list_for_each_safe(pos, q, &read_q->list){
		tmp = list_entry(pos, struct q, list);
	//	printk("r, fd:%d, ts:%ld",cd->fd, tmp->kern_data.timestamp);
		ret = copy_to_user(cd->data, &(tmp->kern_data), sizeof(struct ku_pir_data));
		list_del(pos);
		kfree(tmp);
		break;
	}
	spin_unlock(&my_lock);

	/* unlocking */
	return ret;

}

static int ku_pir_open(struct inode *inode, struct file* file){
	return 0;
}

static int ku_pir_release(struct inode *inode, struct file* file){
	return 0;
}

static long ku_pir_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	long ret;

	ret = 0L;

	switch(cmd){
		case KU_IOCTL_INSERT:
			{
				struct comb_data *cd;
				cd = (struct comb_data*)arg;
				ret = insert_one(cd);
				break;
			}
		case KU_IOCTL_READ:
			{
				struct comb_data *cd;
				cd = (struct comb_data*)arg;
				ret = ku_pir_read(cd);
				break;
			}
		case KU_IOCTL_GET_FD:
			ret = mk_queue();
			break;
		case KU_IOCTL_CLOSE:
			ret = rm_queue(*(int*)arg);
			break;
		case KU_IOCTL_FLUSH:
			ret = flush_queue(*(int*)arg);
			break;
		default:
			break;
	}

	return ret;
};

struct file_operations ku_pir_fops =
{
	.open = ku_pir_open,
	.release = ku_pir_release,
	.unlocked_ioctl = ku_pir_ioctl,
};


static irqreturn_t ku_pir_isr(int irq, void* dev_id){
	int ret;
	char rf_flag;
	long unsigned int timestamp;
	
	ret = 0;
	timestamp = jiffies;
	if(gpio_get_value(KUPIR_SENSOR) == 1)	 /* 1 : rising */
		rf_flag = '0';
	else
		rf_flag = '1';

	insert_all(timestamp, rf_flag);

	return IRQ_HANDLED;
}



static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_pir_init(void){
	int ret;

	printk("Init Module\n");
	
	/*allocate character device*/
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_pir_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);

	if(ret<0){
		printk("fail to add character device\n");
		return -1;
	}

	/* init list, waitqueue, spin_lock */
	INIT_LIST_HEAD(&kern_queues.list);
	init_waitqueue_head(&my_wq);
	spin_lock_init(&my_lock);

	/* init kernel file descriptor */
	kern_fd=0;

	/* requset GPIO and interrupt handler */
	gpio_request_one(KUPIR_SENSOR, GPIOF_IN, "sensor");
	irq_num = gpio_to_irq(KUPIR_SENSOR);
	ret = request_irq(irq_num, ku_pir_isr, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "ku_irq", NULL);
	if(ret){
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		free_irq(irq_num, NULL);
	}

	return ret;
}

static void __exit ku_pir_exit(void){
	struct queues *tmp;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	
	printk("Exit Module \n");
	
	list_for_each_safe(pos, q, &kern_queues.list){
		tmp = list_entry(pos, struct queues, list);
		list_del(pos);
		kfree(tmp);
	}

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
	
	disable_irq(irq_num);
	free_irq(irq_num, NULL);
	gpio_free(KUPIR_SENSOR);
}

module_init(ku_pir_init);
module_exit(ku_pir_exit);

