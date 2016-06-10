#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>

#include "brcm_proc.h"
#include "brcm.h"

/*
  *  Names of the proc directory entries
  */
static const char name_root[]   = "brcm";
static const char name_conf[]   = "config";

/* Methods for preparing data for reading proc entries */
static int brcmdev_seq_show(struct seq_file *seq, void *v);


/* start read of /proc/net/vlan/config */
static void *brcm_seq_start(struct seq_file *seq, loff_t *pos)
  __acquires(rcu)
{
  struct net_device *dev;
  struct net *net = seq_file_net(seq);
  loff_t i = 1;

  rcu_read_lock();
  if (*pos == 0)
    return SEQ_START_TOKEN;

  for_each_netdev_rcu(net, dev) {
    if (!is_brcm_dev(dev))
      continue;

    if (i++ == *pos)
      return dev;
  }

  return  NULL;
}

static void *brcm_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
  struct net_device *dev;
  struct net *net = seq_file_net(seq);

  ++*pos;

  dev = (struct net_device *)v;
  if (v == SEQ_START_TOKEN)
    dev = net_device_entry(&net->dev_base_head);

  for_each_netdev_continue_rcu(net, dev) {
    if (!is_brcm_dev(dev))
      continue;

    return dev;
  }

  return NULL;
}

static void brcm_seq_stop(struct seq_file *seq, void *v)
  __releases(rcu)
{
  rcu_read_unlock();
}

static int brcm_seq_show(struct seq_file *seq, void *v)
{
  if (v == SEQ_START_TOKEN) {
    seq_puts(seq, "BRCM Dev name   | BRCM Port\n");
  } else {
    const struct net_device *brcm_dev = v;
    const struct brcm_dev_info *dev_info = brcm_dev_info(brcm_dev);

    seq_printf(seq, "%-15s| %d  | %s\n",  brcm_dev->name,
         dev_info->brcm_port,    dev_info->real_dev->name);
  }
  return 0;
}

/*
  *  Generic /proc/net/brcm/<file> file and inode operations
  */
static const struct seq_operations brcm_seq_ops = {
  .start = brcm_seq_start,
  .next = brcm_seq_next,
  .stop = brcm_seq_stop,
  .show = brcm_seq_show,
};

static int brcm_seq_open(struct inode *inode, struct file *file)
{
  return seq_open_net(inode, file, &brcm_seq_ops,
      sizeof(struct seq_net_private));
}

static const struct file_operations brcm_fops = {
  .owner   = THIS_MODULE,
  .open    = brcm_seq_open,
  .read    = seq_read,
  .llseek  = seq_lseek,
  .release = seq_release_net,
};

/*
  *  /proc/net/brcm/<device> file and inode operations
  */
static int brcmdev_seq_open(struct inode *inode, struct file *file)
{
  return single_open(file, brcmdev_seq_show, PDE(inode)->data);
}

static const struct file_operations brcmdev_fops = {
  .owner = THIS_MODULE,
  .open    = brcmdev_seq_open,
  .read    = seq_read,
  .llseek  = seq_lseek,
  .release = single_release,
};


void brcm_proc_cleanup(struct net *net)
{
  struct brcm_net *bn = net_generic(net, brcm_net_id);

  if (bn->proc_brcm_conf)
    remove_proc_entry(name_conf, bn->proc_brcm_dir);

  if (bn->proc_brcm_dir)
    proc_net_remove(net, name_root);

  /* Dynamically added entries should be cleaned up as their brcm_device
   * is removed, so we should not have to take care of it here...
   */
}

/*
  *  Add directory entry for BRCM device.
  */
int brcm_proc_add_dev(struct net_device *brcmdev)
{
  struct brcm_dev_info *dev_info = brcm_dev_info(brcmdev);
  struct brcm_net *bn = net_generic(dev_net(brcmdev), brcm_net_id);

  dev_info->dent =
    proc_create_data(brcmdev->name, S_IFREG|S_IRUSR|S_IWUSR,
         bn->proc_brcm_dir, &brcmdev_fops, brcmdev);
  if (!dev_info->dent)
    return -ENOBUFS;
  return 0;
}

/*
  *  Delete directory entry for BRCM device.
  */
int brcm_proc_rem_dev(struct net_device *brcmdev)
{
  struct brcm_net *bn = net_generic(dev_net(brcmdev), brcm_net_id);

  /** NOTE:  This will consume the memory pointed to by dent, it seems. */
  if (brcm_dev_info(brcmdev)->dent) {
    remove_proc_entry(brcm_dev_info(brcmdev)->dent->name,
          bn->proc_brcm_dir);
    brcm_dev_info(brcmdev)->dent = NULL;
  }
  return 0;
}

/*
  *  Create /proc/net/brcm file
  */
int __net_init brcm_proc_init(struct net *net)
{
  struct brcm_net *bn = net_generic(net, brcm_net_id);

  bn->proc_brcm_dir = proc_net_mkdir(net, name_root, net->proc_net);
  if (!bn->proc_brcm_dir)
    goto err;

  bn->proc_brcm_conf = proc_create(name_conf, S_IFREG|S_IRUSR|S_IWUSR,
           bn->proc_brcm_dir, &brcm_fops);
  if (!bn->proc_brcm_conf)
    goto err;
  return 0;

err:
  pr_err("%s: can't create entry in proc filesystem!\n", __func__);
  brcm_proc_cleanup(net);
  return -ENOBUFS;

}

static int brcmdev_seq_show(struct seq_file *seq, void *offset)
{
  struct net_device *brcmdev = (struct net_device *) seq->private;
  const struct brcm_dev_info *dev_info = brcm_dev_info(brcmdev);
  const struct net_device_stats *stats;
  static const char fmt[] = "%30s %12lu\n";

  if (!is_brcm_dev(brcmdev))
    return 0;

  stats = dev_get_stats(brcmdev);
  seq_printf(seq,
       "%s  Port: %d  dev->priv_flags: %hx\n",
       brcmdev->name, dev_info->brcm_port,
       brcmdev->priv_flags);

  seq_printf(seq, fmt, "total frames received", stats->rx_packets);
  seq_printf(seq, fmt, "total bytes received", stats->rx_bytes);
  seq_printf(seq, fmt, "Broadcast/Multicast Rcvd", stats->multicast);
  seq_puts(seq, "\n");
  seq_printf(seq, fmt, "total frames transmitted", stats->tx_packets);
  seq_printf(seq, fmt, "total bytes transmitted", stats->tx_bytes);
  seq_printf(seq, "Device: %s", dev_info->real_dev->name);
  seq_puts(seq, "\n");

  return 0;
}

