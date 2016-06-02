#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/gpio.h>


#define DEVICE_NAME "mymap"


static unsigned char array[10]={0,1,2,3,4,5,6,7,8,9};
static unsigned char *buffer;


static int my_open(struct inode *inode, struct file *file)
{
    return 0;
}


static int my_map(struct file *filp, struct vm_area_struct *vma)
{    
    unsigned long page;
    unsigned char i;
    unsigned long start = (unsigned long)vma->vm_start;
    //unsigned long end =  (unsigned long)vma->vm_end;
    unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);

    //\u5f97\u5230\u7269\u7406\u5730\u5740
    page = virt_to_phys(buffer);    
    //\u5c06\u7528\u6237\u7a7a\u95f4\u7684\u4e00\u4e2avma\u865a\u62df\u5185\u5b58\u533a\u6620\u5c04\u5230\u4ee5page\u5f00\u59cb\u7684\u4e00\u6bb5\u8fde\u7eed\u7269\u7406\u9875\u9762\u4e0a
    if(remap_pfn_range(vma,start,page>>PAGE_SHIFT,size,PAGE_SHARED))//\u7b2c\u4e09\u4e2a\u53c2\u6570\u662f\u9875\u5e27\u53f7\uff0c\u7531\u7269\u7406\u5730\u5740\u53f3\u79fbPAGE_SHIFT\u5f97\u5230
        return -1;

    //\u5f80\u8be5\u5185\u5b58\u519910\u5b57\u8282\u6570\u636e
    for(i=0;i<10;i++)
        buffer[i] = array[i];
    
    return 0;
}


static struct file_operations dev_fops = {
    .owner    = THIS_MODULE,
    .open    = my_open,
    .mmap   = my_map,
};

static struct miscdevice misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &dev_fops,
};


static int __init dev_init(void)
{
    int ret;    

    //\u6ce8\u518c\u6df7\u6742\u8bbe\u5907
    ret = misc_register(&misc);
    //\u5185\u5b58\u5206\u914d
    buffer = (unsigned char *)kmalloc(PAGE_SIZE,GFP_KERNEL);
    //\u5c06\u8be5\u6bb5\u5185\u5b58\u8bbe\u7f6e\u4e3a\u4fdd\u7559
    SetPageReserved(virt_to_page(buffer));

    return ret;
}


static void __exit dev_exit(void)
{
    //\u6ce8\u9500\u8bbe\u5907
    misc_deregister(&misc);
    //\u6e05\u9664\u4fdd\u7559
    ClearPageReserved(virt_to_page(buffer));
    //\u91ca\u653e\u5185\u5b58
    kfree(buffer);
}


module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LKN@SCUT");