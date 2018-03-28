#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/cdev.h>
MODULE_LICENSE("GPL");

#define DEV_NAME "mod1_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1

#define SIMPLE_IOCTL_NUM 'z'
#define SIMPLE_INCREASE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)

int my_var;
EXPORT_SYMBOL(my_var);

static long mod1_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	switch( cmd ){
	case SIMPLE_INCREASE:
		printk("Mod1 : Increase my_var \n");
		my_var++;
		break;
	default:
		return -1;
	}

	return 0;
}

static int mod1_ioctl_open(struct inode *inode, struct file *file){
	printk("mod1_ioctl open\n");
	return 0;
}

static int mod1_ioctl_release(struct inode *inode, struct file *file){
	printk("mod1_ioctl release\n");
	return 0;
}

struct file_operations mod1_char_fops =
{
	.unlocked_ioctl = mod1_ioctl,
	.open = mod1_ioctl_open,
	.release = mod1_ioctl_release,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init mod1_ioctl_init(void){
	printk("Init Module 1\n");
	my_var=0;

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &mod1_char_fops);
	cdev_add(cd_cdev, dev_num, 1);

	return 0;
}

static void __exit mod1_ioctl_exit(void){
	printk("Exit Module 1\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(mod1_ioctl_init);
module_exit(mod1_ioctl_exit);

