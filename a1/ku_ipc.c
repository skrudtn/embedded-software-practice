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

struct q* getQueue(int key);

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
//struct q my_q;

void delay(int sec){
	int i, j;
	for(j=0; j<sec; j++){
		for(i=0; i<1000; i++){
			udelay(1000);
		}
	}
}

int isExistKey(int *key){
	printk("called isExistKey [%d]\n", *key);
	struct queues *tmp = 0;
	int ret=0;
	list_for_each_entry(tmp, &kern_queues.list, list){
		printk("queue's key [%d]\n", tmp->key);
		if(tmp->key == *key)
			ret = tmp->key;
	}
	return ret;
}

int createQueue(int *key){
	printk("create Queue : %d", *key);
	struct queues *new_queues = 0;
	struct list_head *pos = 0;
	//struct q new_q;
	//INIT_LIST_HEAD(&new_q.list);

//	INIT_LIST_HEAD(&new_queues.list);
	new_queues = (struct queues*)kmalloc(sizeof(struct queues), GFP_KERNEL);
	new_queues->key = *key;
	INIT_LIST_HEAD(&new_queues->kern_q.list);
//	new_queues->kern_q = new_q;
	
	
	list_add(&new_queues->list, &kern_queues.list);
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

struct q* getQueue(int key){
	int i=0;
	int ret = 0;
	struct queues *tmp = 0;
	struct q *retQ = 0;
	printk("getQueue : %d", key);

	list_for_each_entry(tmp, &kern_queues.list, list){
		if(tmp->key == key){
			retQ = &tmp->kern_q;
			printk("idx [%d], key[%d] \n", i, tmp->key);
		}
		i++;
	}
	return retQ;
}

void displayQueue(int key){
	int i=0;
	struct q *tmp = 0;
	struct q *displayedQ = getQueue(key);

	list_for_each_entry(tmp, &displayedQ->list, list){
		printk("pos[%d], type[%ld], text[%s] \n", i, tmp->kern_buf.type, tmp->kern_buf.text);
		i++;
	}

}

static int ku_ipc_read(struct ipcbuf *ipc_buf){
	int ret;
	unsigned int i = 0;
	struct q *temp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	struct q *read_q = 0;
		
	read_q = getQueue(ipc_buf->msqid);

	delay(1);

	spin_lock(&s_lock);
	list_for_each_safe(pos, q, &read_q->list){
		if(i==0){
			temp = list_entry(pos, struct q, list);
			ret = copy_to_user(ipc_buf->msgp, &temp->kern_buf, sizeof(temp->kern_buf));
			printk("read kern : %s\n", temp->kern_buf.text);
			printk("read buf : %s\n", ((struct msgbuf*)ipc_buf->msgp)->text);
			list_del(pos);
			kfree(temp);
		}
		i++;
	}
	spin_unlock(&s_lock);
	
	displayQueue(ipc_buf->msqid);

	return ret;
}

static int ku_ipc_write(struct ipcbuf *ipc_buf){
	int ret;
	struct q *temp = 0;
	struct q *written_q = 0;
	struct msgbuf *user_msgbuf;
	
	user_msgbuf = (struct msgbuf*)ipc_buf->msgp;
	written_q = getQueue(ipc_buf->msqid);

	temp = (struct q*)kmalloc(sizeof(struct q), GFP_KERNEL);
	printk("%s\n", user_msgbuf->text);
	printk("%ld\n", user_msgbuf->type);

	printk("write buf before : %s\n", user_msgbuf->text);
	ret = copy_from_user(&temp->kern_buf, user_msgbuf, ipc_buf->msgsz);
	printk("write kern after : %s\n", temp->kern_buf.text);
	spin_lock(&s_lock);
	list_add_tail(&temp->list, &written_q->list);
	spin_unlock(&s_lock);

	displayQueue(ipc_buf->msqid);
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
			ret = isExistKey((int*)arg);
			break;
		case KU_IOCTL_CREATE_QUEUE:
			ret = createQueue((int*)arg);
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
	struct list_head *pos = 0;
	struct list_head *q = 0;

	printk("Exit Module\n");
	list_for_each_safe(pos, q, &kern_queues.list){
		list_del(pos);
	}

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);
