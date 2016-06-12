#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/signal.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>

static int major = 250;
static int minor=0;
static dev_t devno;
static struct class *cls;
static struct device *test_device;

static char temp[64]={0};
static struct fasync_struct *fasync;

static int hello_open (struct inode *inode, struct file *filep)
{
	return 0;
}
static int hello_release(struct inode *inode, struct file *filep)
{
	return 0;
}

static ssize_t hello_read(struct file *filep, char __user *buf, size_t len, loff_t *pos)
{
	if(len>64)
	{
		len =64;
	}
	if(copy_to_user(buf,temp,len))
	{
		return -EFAULT;
	}	
	return len;
}
static ssize_t hello_write(struct file *filep, const char __user *buf, size_t len, loff_t *pos)
{
	if(len>64)
	{
		len = 64;
	}

	if(copy_from_user(temp,buf,len))
	{
		return -EFAULT;
	}
	printk("write %s\n",temp);

	kill_fasync(&fasync, SIGIO, POLL_IN);
	return len;
}

static int hello_fasync (int fd, struct file * file, int on)
{
	return fasync_helper(fd, file, on, &fasync);

}
static struct file_operations hello_ops=
{
	.open = hello_open,
	.release = hello_release,
	.read =hello_read,
	.write=hello_write,
	.fasync = hello_fasync
};
static int hello_init(void)
{
	int ret;	
	devno = MKDEV(major,minor);
	ret = register_chrdev(major,"hello",&hello_ops);

	cls = class_create(THIS_MODULE, "myclass");
	if(IS_ERR(cls))
	{
		unregister_chrdev(major,"hello");
		return -EBUSY;
	}
	test_device = device_create(cls,NULL,devno,NULL,"hello");//mknod /dev/hello
	if(IS_ERR(test_device))
	{
		class_destroy(cls);
		unregister_chrdev(major,"hello");
		return -EBUSY;
	}	
	return 0;
}
static void hello_exit(void)
{
	device_destroy(cls,devno);
	class_destroy(cls);	
	unregister_chrdev(major,"hello");
	printk("hello_exit \n");
}
MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);

