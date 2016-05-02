#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h> 
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/slab.h>
//#include <linux/sched.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/uaccess.h>

#define VIRTUALDISK_SIZE  0x1000//4k
#define MEM_CLEAR 0x1
#define VIRTUALDISK_MAJOR 250
int VirtualDisk_major = VIRTUALDISK_MAJOR;
struct VirtualDisk{
    struct cdev cdev;
    unsigned char mem[VIRTUALDISK_SIZE];
    long count;          
};
static struct class *module_class;
struct VirtualDisk *VirtualDiskp;
static int module_drv_open(struct inode *inode, struct file *file)
{
    printk("module_dev read\n");
    file->private_data = VirtualDiskp;
    VirtualDiskp->count++;    /*add device count*/
    return 0;
}
static int module_drv_release(struct inode *inode, struct file *file)
{
    printk("module_dev release\n");
    VirtualDiskp->count--;  /*reduce device count*/
    return 0;
}

static loff_t module_drv_llseek(struct file *file, loff_t offset, int origin){

  loff_t ret = 0;
                                                                                                                                                                                                                                                                                                                                                                                                                
  switch (origin)
  {
    case SEEK_SET:   
      if (offset < 0)
      {
        ret =  - EINVAL; 
        break;
      }
      if ((unsigned int)offset > VIRTUALDISK_SIZE)
      {
        ret =  - EINVAL;
        break;
      }
      file->f_pos = (unsigned int)offset;
      ret = file->f_pos;
      break;
    case SEEK_CUR:
      if ((file->f_pos + offset) > VIRTUALDISK_SIZE)
      {
        ret =  - EINVAL;
        break;
      }
      if ((file->f_pos + offset) < 0)
      {
        ret =  - EINVAL;
        break;
      }
      file->f_pos += offset;
      ret = file->f_pos;
      break;
    default:
      ret =  - EINVAL;
      break;
  }
  return ret;
}


static long module_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
     struct VirtualDisk *devp = file->private_data;
                                                                                                                                                                                                                                                                                                                                                                                                                  
    switch (cmd)
    {
    case MEM_CLEAR:
      memset(devp->mem, 0, VIRTUALDISK_SIZE);
      printk(KERN_INFO "VirtualDisk is set to zero\n");
      break;
    default:
      return  - EINVAL;
    }
    return 0;
}

static ssize_t module_drv_read(struct file *file, char __user *buf, size_t count, loff_t * ppos)
{                                                                                                                                                                                                                                                                                                                                                                                                                
  unsigned long p =  *ppos; 
  unsigned int countt = count;
  int ret = 0;
  struct VirtualDisk *devp = file->private_data; 
  printk("module_dev read\n");

  if (p >= VIRTUALDISK_SIZE)  
    return countt ?  - ENXIO: 0;
  if (countt > VIRTUALDISK_SIZE  - p)
    countt = VIRTUALDISK_SIZE  - p;

  if (copy_to_user(buf, (void*)(devp->mem + p), countt))
  {
    ret =  - EFAULT;
  }
  else
  {
    *ppos += countt;
    ret = countt;
    printk("read %d bytes(s) is  %ld\n", countt, p);
  }
    printk("bytes(s) is  %s\n", buf);
  return ret;
}


static ssize_t module_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{                                                                                                                                                                                                                                                                                                                                                                                                                  
  unsigned long p =  *ppos;
  int ret = 0; 
  unsigned int countt = count;
  struct VirtualDisk *devp = file->private_data; 
 printk("module_dev write\n");

  if (p >= VIRTUALDISK_SIZE )
    return countt ?  - ENXIO: 0;
  if (countt > VIRTUALDISK_SIZE  - p)
    countt = VIRTUALDISK_SIZE  - p;

  if (copy_from_user(devp->mem + p, buf, countt))
    ret =  - EFAULT;
  else
  {
    *ppos += countt;
    ret = countt;
    printk("written %d bytes(s) from%ld\n", countt, p);
  }
  return ret;
    return 0;
}
static int memdev_mmap(struct file*file, struct vm_area_struct *vma){
    struct VirtualDisk *devp = file->private_data; 
    vma->vm_flags |= VM_IO;
    vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);
    if (remap_pfn_range(vma,vma->vm_start,virt_to_phys(devp->mem)>>PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot))
          return  -EAGAIN;
                                                                                                                                                                                                                                                                                                                                                                                                                               
      return 0;
}
static struct file_operations module_drv_fops = {
    .owner  =   THIS_MODULE,    
    .open   =   module_drv_open,
    .read = module_drv_read,
    .write  =   module_drv_write,
    .release = module_drv_release,
    .llseek = module_drv_llseek,
    .unlocked_ioctl = module_drv_ioctl,
    .mmap = memdev_mmap,
};

static void VirtualDisk_setup_cdev(struct VirtualDisk *dev, int minorIndex){
    int err;
    int devno = MKDEV(VirtualDisk_major, minorIndex);
    cdev_init(&dev->cdev, &module_drv_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    if(err){
      printk("error %d cdev file added\n", err);
    }
}
static int module_drv_init(void)
{
    int result;
    dev_t devno = MKDEV(VirtualDisk_major, 0);
    if(VirtualDisk_major){
    result = register_chrdev_region(devno, 1, "module");
    }else{
    result = alloc_chrdev_region(&devno, 0, 1, "module");
    VirtualDisk_major = MAJOR(devno);
    }
    if(result < 0 ){
    return result;
    }
    VirtualDiskp = kmalloc(sizeof(struct VirtualDisk), GFP_KERNEL);
    if(!VirtualDiskp){
    result = -ENOMEM;
    goto fail_malloc;
    }
    memset(VirtualDiskp, 0, sizeof(struct VirtualDisk));
    VirtualDisk_setup_cdev(VirtualDiskp, 0);
    if (IS_ERR(module_class))
        return PTR_ERR(module_class);
    return 0;
fail_malloc:
    unregister_chrdev_region(devno, 1);
    return result;                                                                                                                                                                                                                                                                                                                                                                                                                   
}
static void module_drv_exit(void)
{
    cdev_del(&VirtualDiskp->cdev);
    kfree(VirtualDiskp);
    unregister_chrdev_region(MKDEV(VirtualDisk_major, 0), 1);
}
module_init(module_drv_init);
module_exit(module_drv_exit);
MODULE_LICENSE("GPL");

