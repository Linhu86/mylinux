/* tanglinux.c */  
#include <linux/module.h>  
#include <linux/types.h>  
#include <linux/miscdevice.h>  
#include <linux/fs.h>  
#include <linux/netdevice.h>  
#include <linux/etherdevice.h>  
#include <linux/kernel.h>  
#include <linux/ioctl.h>  
  
#define TANGLINUX _IO('T', 1)  
  
struct net_local {  
    int count;  
    char ch;  
};  
  
static int tanglinux_open(struct inode *inode, struct file *file)  
{  
    return nonseekable_open(inode, file);  
}  
  
static long tanglinux_ioctl(struct file *file, unsigned int cmd, unsigned long arg)  
{  
    struct net_device *dev;  
    size_t alloc_size;  
    size_t sizeof_priv = sizeof(struct net_local);  
    struct net_device *p;  
      
    switch (cmd) {  
    case TANGLINUX:  
        alloc_size = sizeof(struct net_device);  
        printk("first: alloc_size = %d\n", alloc_size);  
  
        alloc_size += 1; // do not align size.
  
        if (sizeof_priv) {  
        /* ensure 32-byte alignment of private area */  
        alloc_size = ALIGN(alloc_size, NETDEV_ALIGN); //#define NETDEV_ALIGN    32  
        printk("second: alloc_size = %d\n", alloc_size);  
  
        alloc_size += sizeof_priv;  
        printk("third: alloc_size = %d\n", alloc_size);  
        }  
        /* ensure 32-byte alignment of whole construct */  
        alloc_size += NETDEV_ALIGN - 1;  
        printk("fourth: alloc_size = %d\n", alloc_size);  
      
        p = kzalloc(alloc_size, GFP_KERNEL);
        if (!p) {  
        printk(KERN_ERR "alloc_netdev: Unable to allocate device.\n");  
        return -ENOMEM;  
        }  
        printk("p = %p\n", p);  
      
        dev = PTR_ALIGN(p, NETDEV_ALIGN);  
        printk("dev = %p\n", dev);  
  
        dev->padded = (char *)dev - (char *)p;  
      
        printk("dev->padded = %d\n", dev->padded);  
  
        kfree(p);  
  
        return 0;  
    default:  
        return -ENOTTY;  
    }  
}  
  
static int tanglinux_release(struct inode *inode, struct file *file)  
{  
    return 0;  
}  
  
static const struct file_operations tanglinux_fops = {  
    .owner          = THIS_MODULE,  
    .unlocked_ioctl = tanglinux_ioctl,  
    .open           = tanglinux_open,  
    .release        = tanglinux_release,  
};  
  
static struct miscdevice tanglinux_miscdev = {  
    .minor  = WATCHDOG_MINOR,  
    .name   = "tanglinux",  
    .fops   = &tanglinux_fops,  
};  
  
static int __init tanglinux_init(void)  
{  
    printk("tanglinux driver\n");  
  
    return misc_register(&tanglinux_miscdev);  
}  
  
static void __exit tanglinux_exit(void)  
{  
    misc_deregister(&tanglinux_miscdev);  
}  
  
module_init(tanglinux_init);  
module_exit(tanglinux_exit);  
  
MODULE_LICENSE("GPL"); 




