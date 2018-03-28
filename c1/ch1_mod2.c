#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/cdev.h>
MODULE_LICENSE("GPL");

#define DEV_NAME "mod2_dev"

extern int my_var;

static int mod2_read(struct file *file, char *buf, size_t len, loff_t *lof){
	printk("my_var : %d\n",my_var);
	return 0;
}


static int mod2_open(struct inode *inode, struct file *file){
	printk("mod2 open\n");
	return 0;
}

static int mod2_release(struct inode *inode, struct file *file){
	printk("mod2 release\n");
	return 0;
}

struct file_operations mod2_char_fops =
{
	.read = mod2_read,
	.open = mod2_open,
	.release = mod2_release,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init mod2_init(void){
	printk("Init Module 2\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &mod2_char_fops);
	cdev_add(cd_cdev, dev_num, 1);

	return 0;
}

static void __exit mod2_exit(void){
	printk("Exit Module 2\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(mod2_init);
module_exit(mod2_exit);

