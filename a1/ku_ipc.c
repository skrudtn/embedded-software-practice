#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/delay.h>
MODULE_LICENSE("GPL");

#include "ku_ipc.h"

struct q* get_queue(int key);

spinlock_t s_lock;

struct q{
	struct list_head list;
	struct msgbuf kern_buf;
};

struct queues{
	struct list_head list;
	struct q kern_q;
	int key;
};


struct queues kern_queues;

void delay(int sec){
	int i, j;
	for(j=0; j<sec; j++){
		for(i=0; i<1000; i++){
			udelay(1000);
		}
	}
}

int is_exist_key(int *key){
	//	printk("called isExistKey [%d]\n", *key);
	struct queues *tmp = 0;
	int ret=-1;

	spin_lock(&s_lock);
	list_for_each_entry(tmp, &kern_queues.list, list){
		printk("is exist key [%d]\n", tmp->key);
		if(tmp->key == *key)
			ret = tmp->key;
	}
	spin_unlock(&s_lock);
	
	return ret;
}

int mk_queue(int *key){
	//	printk("create Queue : %d", *key);
	struct queues *new_queues = 0;
	new_queues = (struct queues*)kmalloc(sizeof(struct queues), GFP_KERNEL);
	new_queues->key = *key;
	INIT_LIST_HEAD(&new_queues->kern_q.list);
	
	spin_lock(&s_lock);
	list_add(&new_queues->list, &kern_queues.list);
	spin_unlock(&s_lock);
	/*
	   struct queues *tmp=0;
	   struct q *tmpQ=0;
	   int i=0;
	   list_for_each_entry(tmp, &kern_queues.list, list){
	   printk("idx [%d], key [%d] \n", i, tmp->key);
	   tmpQ=getQueue(tmp->key);
	   i++;
	   }

	   struct q *tQ = 0;
	   for(i=0; i<5; i++){
	   tQ = (struct q*)kmalloc(sizeof(struct q), GFP_KERNEL);
	   tQ->kern_buf.type = (long)i;
	   list_add(&tQ->list, &tmpQ->list);
	   }

	   list_for_each_entry(tQ, &tmpQ->list, list){
	   printk("%ld",tQ->kern_buf.type);
	   }
	 */
	return new_queues->key;
}

int rm_queue(int *msqid){
	printk("remove Queue called : %d", *msqid);
	struct queues *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *next_q = 0;
	int ret = -1;

	spin_lock(&s_lock);
	list_for_each_safe(pos, next_q, &kern_queues.list){
		tmp = list_entry(pos, struct queues, list);
		if(tmp->key == *msqid){
			printk("removed queue key : %d",tmp->key);
			list_del(pos);
			kfree(tmp);
			ret = 0;
		}
	}
	spin_unlock(&s_lock);

	return ret;
}


struct q* get_queue(int key){
	int i=0;
	int ret = 0;
	struct queues *tmp = 0;
	struct q *retQ = 0;
	printk("getQueue : %d", key);
	
	spin_lock(&s_lock);
	list_for_each_entry(tmp, &kern_queues.list, list){
		if(tmp->key == key){
			retQ = &tmp->kern_q;
			printk("idx [%d], key[%d] \n", i, tmp->key);
		}
		i++;
	}
	spin_unlock(&s_lock);

	return retQ;
}

int is_full_queue(int *msqid){
	int ret=0;
	struct q *tmp = get_queue(*msqid);
	printk("sizeof(queue) %d", sizeof(*tmp));
	if(get_queue_size(*msqid) == KUIPC_MAXVOL)
		ret = 1;
	return ret;
}

int get_queue_size(int *msqid){
	unsigned int ret;
	struct q *tmp = 0;
	struct q *target_q = get_queue(*msqid);

	list_for_each_entry(tmp, &target_q->list, list){
		//tmp->
	}
}

void displayQueue(int key){
	printk("%d",key);
	int i=0;
	struct q *tmp = 0;
	struct q *displayedQ = get_queue(key);

	list_for_each_entry(tmp, &displayedQ->list, list){
		printk("pos[%d], type[%ld], text[%s] \n", i, tmp->kern_buf.type, tmp->kern_buf.text);
		i++;
	}

}

static int ku_ipc_read(struct ipcbuf *ipc_buf){
	int ret;
	unsigned int i = 0;
	struct q *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	struct q *read_q = 0;
	long type = 0L;
	read_q = get_queue(ipc_buf->msqid);

	//delay(1);

	spin_lock(&s_lock);
	list_for_each_safe(pos, q, &read_q->list){
		tmp = list_entry(pos, struct q, list);
		type = ipc_buf->msgtyp;
		if(type == 0) {
			if(i==0){
				ret = copy_to_user(ipc_buf->msgp, &tmp->kern_buf, sizeof(tmp->kern_buf));
				printk("read kern : %s\n", tmp->kern_buf.text);
				printk("read buf : %s\n", ((struct msgbuf*)ipc_buf->msgp)->text);
				list_del(pos);
				kfree(tmp);
			}
			i++;
		} else if(type>0){
			if(type == tmp->kern_buf.type){
				if(i==0){
					ret = copy_to_user(ipc_buf->msgp, &tmp->kern_buf, sizeof(tmp->kern_buf));
					list_del(pos);
					kfree(tmp);
				}
				i++;
			}
		} else if(type<0){
			if(~type+1 >= tmp->kern_buf.type){
				if(i==0){
					ret = copy_to_user(ipc_buf->msgp, &tmp->kern_buf, sizeof(tmp->kern_buf));
					list_del(pos);
					kfree(tmp);
				}
				i++;
			}

		}
	}
	spin_unlock(&s_lock);

	displayQueue(ipc_buf->msqid);

	return ret;
}

static int ku_ipc_write(struct ipcbuf *ipc_buf){
	int ret;
	struct q *tmp = 0;
	struct q *written_q = 0;
	struct msgbuf *user_msgbuf;
	printk("%ld",ipc_buf->msqid);
	user_msgbuf = (struct msgbuf*)ipc_buf->msgp;
	written_q = get_queue(ipc_buf->msqid);
	
	tmp = (struct q*)kmalloc(sizeof(struct q), GFP_KERNEL);
	printk("%s\n", user_msgbuf->text);
	printk("%ld\n", user_msgbuf->type);

	printk("write buf before : %s\n", user_msgbuf->text);
	ret = copy_from_user(&tmp->kern_buf, user_msgbuf, ipc_buf->msgsz);
	printk("write kern after : %s\n", tmp->kern_buf.text);
	printk("%p", written_q);
	spin_lock(&s_lock);
	list_add_tail(&tmp->list, &written_q->list);
	spin_unlock(&s_lock);

	//displayQueue(ipc_buf->msqid);
	return ret;
}

static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	struct msgbuf *user_buf;
	int ret;

	ret = 0;
	user_buf = (struct msgbuf*)arg;

	switch(cmd){
		case KU_IOCTL_RCV:
			{
				struct ipcbuf *ipc_buf;
				ipc_buf = (struct ipcbuf*)arg;
				printk("ioctl : %ld",ipc_buf->msgtyp);
				ret = ku_ipc_read(ipc_buf);
				break;
			}
		case KU_IOCTL_SND:
			{
				struct ipcbuf *ipc_buf;
				ipc_buf = (struct ipcbuf*)arg;
				ret = ku_ipc_write(ipc_buf);
				break;
			}
		case KU_IOCTL_IS_EXIST_KEY:
			ret = is_exist_key((int*)arg);
			break;
		case KU_IOCTL_CREATE_QUEUE:
			ret = mk_queue((int*)arg);
			break;
		case KU_IOCTL_MSGCLOSE:
			ret = rm_queue((int*)arg);
			break;
		case KU_IOCTL_IS_FULL_QUEUE:
			ret = is_full_queue((int*)arg);
			break;
	
	}

	return ret;
}

static int ku_ipc_open(struct inode *inode, struct file *file) {
	printk("ku_ipc open\n");
	return 0;
}

static int ku_ipc_release(struct inode *inode, struct file *file) {
	printk("ku_ipc release\n");
	return 0;
}

struct file_operations ku_ipc_fops = {
	.unlocked_ioctl = ku_ipc_ioctl,
	.open = ku_ipc_open,
	.release = ku_ipc_release
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_ipc_init(void) {
	int ret;

	printk("Init Module\n");
	INIT_LIST_HEAD(&kern_queues.list);

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_ipc_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);
	if(ret <0){
		printk("fail to add character device\n");
		return -1;
	}

	return ret;
}

static void __exit ku_ipc_exit(void){
	struct queues *tmp;
	struct list_head *pos = 0;
	struct list_head *q = 0;

	printk("Exit Module\n");
	list_for_each_safe(pos, q, &kern_queues.list){
		tmp = list_entry(pos, struct queues, list);
		list_del(pos);
		kfree(tmp);
	}

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);
