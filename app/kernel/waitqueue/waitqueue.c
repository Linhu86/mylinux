#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linhu");
MODULE_DESCRIPTION("A simple BLOCKING IO example");

static wait_queue_head_t my_wait_queue;

//static struct work_struct workq;

static char *ptr_buffer = NULL;

static struct semaphore sem;


static int test_proc_show(struct seq_file *ptr,void * param);
static int test_proc_open(struct inode *inode, struct file *file);
static ssize_t test_proc_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos);
static ssize_t test_proc_write(struct file *file, const char __user *buf, size_t nbytes, loff_t *ppos);

/*
struct chardev{
  struct semaphore sem;
  struct cdev cdev;
}chardev_t;


static struct file_operations char_ops = {
  .owner = THIS_MODULE,
  .open  = test_open,
  .
};
*/

static struct file_operations proc_ops = {
  .owner = THIS_MODULE,
  .open  = test_proc_open,
  .read  = test_proc_read,
  .write = test_proc_write,
  .release = single_release
};


static int test_proc_show(struct seq_file *ptr,void * param)
{
  printk(KERN_INFO "test_proc opened.\n");
  return 0;
}

static int test_proc_open(struct inode *inode, struct file *file)
{
  return single_open(file, test_proc_show, NULL);
}

static ssize_t test_proc_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
  ssize_t rbytes = 0;

  if(ptr_buffer == NULL)
  {
    printk(KERN_INFO "Buffer is NULL.\n");
    return EFAULT;
  }

  if(strlen(ptr_buffer) == 0)
  {
    printk(KERN_INFO "Buffer is not available, wait until data is available.\n");
    interruptible_sleep_on(&my_wait_queue);
  }

  down_interruptible(&sem);

  rbytes = copy_to_user(buf, ptr_buffer, strlen(ptr_buffer));

  up(&sem);

  if(rbytes == 0)
  {
    printk(KERN_ERR "Failed to copy to user.\n");
    return EFAULT;
  }

  printk(KERN_INFO "read data: %s\n", ptr_buffer);

  memset(ptr_buffer, 0, strlen(ptr_buffer));
  
  return rbytes;
}


static ssize_t test_proc_write(struct file *file, const char __user *buf, size_t nbytes, loff_t *ppos)
{

  down_interruptible(&sem);

  if(copy_from_user(ptr_buffer, buf, nbytes) < 0)
  {
    printk(KERN_ERR "Failed to copy from user.\n");
    return EFAULT;
  }

  up(&sem);

  printk(KERN_ERR "Wake up to read the buffer.\n");
  wake_up_interruptible(&my_wait_queue);
  return nbytes;
}


/*
void my_workqueue_handler(struct work_struct *work)
{
  printk("WORK QUEUE: I'm just a timer to wake up the sleeping module.\n");
  msleep(1000);
  printk("WORK QUEUE: Time up Module wake up.\n");
  wake_up_interruptible(&my_wait_queue);
}
*/
int init_module(void)
{
  printk("Wait queue create buffer...\n");

  ptr_buffer = kmalloc(1024*sizeof(char), GFP_KERNEL);

//  INIT_WORK(&workq, my_workqueue_handler);

//  schedule_work(&workq);

  sema_init(&sem, 1);

  init_waitqueue_head(&my_wait_queue);

  proc_create("Waitq", 0, NULL, &proc_ops);

  return 0;
}

void cleanup_module(void)
{
  remove_proc_entry("Waitq", NULL);
  kfree(ptr_buffer);
  printk("<1> Start to cleanup \n");
}



