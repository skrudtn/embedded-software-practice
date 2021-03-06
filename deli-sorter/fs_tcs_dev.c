#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include "fs_tcs.h"


MODULE_LICENSE("GPL");

static int tcs_open(struct inode *inode, struct file *f) {

	struct tcs_dev *tcs;

	tcs = container_of(inode->i_cdev, struct tcs_dev, cdev);
	f->private_data = tcs; 

	return 0;
}

static int tcs_close(struct inode *i, struct file *f) {
	struct tcs_dev *tcs = f->private_data;

	tcs_stop_measurement(tcs);
	return 0;
}

static ssize_t tcs_read(struct file *f, char __user *buf, size_t count, loff_t *off) {
	DECLARE_WAITQUEUE(wait, current);
	struct tcs_dev *tcs = f->private_data;
	size_t max = sizeof(struct tcs3200_measurement);

	if (*off >= max || !count)
		return 0;

	tcs_start_measurement(tcs);
	add_wait_queue(&tcs->waitq, &wait);
	while(1) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if(signal_pending(current)) {
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&tcs->waitq, &wait);
			tcs_stop_measurement(tcs);
			return -ERESTARTSYS;
		}
		if(tcs->state == READ_DONE) {
			tcs_stop_measurement(tcs);
			break;
		}
	}
	remove_wait_queue(&tcs->waitq, &wait);

	if(*off + count > max)
		count = max - *off;

	if(copy_to_user(buf, &tcs->measurement + *off, count))
		return -EFAULT;

	printk("%u,%u,%u,%u\n",tcs->measurement.white_head, tcs->measurement.red, tcs->measurement.green, tcs->measurement.blue);
	return count;
}

static ssize_t tcs_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {

	return -EINVAL;
}

static struct file_operations tcs_fops = {
	.open = tcs_open,
	.release = tcs_close,
	.read = tcs_read,
	.write = tcs_write
};

static struct tcs_dev *tcs;

static int __init tcs3200_init(void) {
	int rc;

	tcs = kzalloc(sizeof(struct tcs_dev), GFP_KERNEL);
	alloc_chrdev_region(&tcs->devid, 0, 1, DEV_NAME);
	cdev_init(&tcs->cdev, &tcs_fops);
	cdev_add(&tcs->cdev, tcs->devid, 1);

	init_waitqueue_head(&tcs->waitq);
	
	tcs_control_init(tcs);
	tcs_counter_init(tcs);
	return 0;

}

static void __exit tcs3200_free(void) {
	tcs_counter_exit(tcs);
	tcs_control_exit();
	cdev_del(&tcs->cdev);
	unregister_chrdev_region(tcs->devid, 1);
	kfree(tcs);
	printk(KERN_INFO "tcs3200 removed\n");
}

module_init(tcs3200_init);
module_exit(tcs3200_free);


