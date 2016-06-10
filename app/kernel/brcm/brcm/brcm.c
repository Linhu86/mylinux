#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <net/rtnetlink.h>
#include <net/netns/generic.h>
#include <linux/etherdevice.h>

#include "brcm.h"
#include "brcm_proc.h"

#define DRV_VERSION "0.1"

int brcm_net_id __read_mostly;

static struct hlist_head brcm_group_hash[BRCM_GRP_HASH_SIZE];

const char brcm_fullname[] = "BRCM Support";
const char brcm_version[] = DRV_VERSION;
static const char brcm_copyright[] = "yoyo <qy5328464@gmail.com>";
static const char brcm_buggyright[] = "yoyo <qy5328464@gmail.com>";

static int __net_init brcm_init_net(struct net *net)
{
  int err;
  err = brcm_proc_init(net);
  return err;
}

static void __net_exit brcm_exit_net(struct net *net)
{
  brcm_proc_cleanup(net);
}

static struct pernet_operations brcm_net_ops = {
  .init = brcm_init_net,
  .exit = brcm_exit_net,
  .id = &brcm_net_id,
  .size = sizeof(struct brcm_net),
};

static inline unsigned int brcm_grp_hashfn(unsigned int idx)
{
  return ((idx >> BRCM_GRP_HASH_SHIFT) ^ idx) & BRCM_GRP_HASH_MASK;
}

static struct brcm_group *brcm_find_group(struct net_device *real_dev)
{
  struct brcm_group *grp;
  struct hlist_node *n;
  int hash = brcm_grp_hashfn(real_dev->ifindex);

  hlist_for_each_entry_rcu(grp, n, &brcm_group_hash[hash], hlist) {
  if (grp->real_dev == real_dev)
    return grp;
  }

  return NULL;
}

static void brcm_group_free(struct brcm_group *grp)
{
  kfree(grp);
}

static struct brcm_group *brcm_group_alloc(struct net_device *real_dev)
{
  struct brcm_group *grp;
  
  grp = kzalloc(sizeof(struct brcm_group), GFP_KERNEL);
  if (!grp)
    return NULL;
  
  grp->real_dev = real_dev;
  hlist_add_head_rcu(&grp->hlist, &brcm_group_hash[brcm_grp_hashfn(real_dev->ifindex)]);
  return grp;
}

/* if brcm device reference to 'brcm_port' not created, return NULL */
static inline struct net_device *brcm_group_get_device(struct brcm_group *bg, u16 brcm_port)
{
  if (bg)
    return bg->brcm_devices_array[brcm_port];
  return NULL;
}

static inline void brcm_group_set_device(struct brcm_group *bg, u16 brcm_port, struct net_device *dev)
{
  if (!bg)
    return;
  bg->brcm_devices_array[brcm_port] = dev;
}

struct net_device *get_brcm_dev(struct net_device *real_dev, u16 brcm_port)
{
  struct brcm_group *grp = brcm_find_group(real_dev);
  if (grp)
    return grp->brcm_devices_array[brcm_port];
  return NULL;
}

struct net_device *find_brcm_dev(struct net_device *real_dev, u16 brcm_port)
{
  struct brcm_group *grp = brcm_find_group(real_dev);
  struct net_device *brcm_dev;
  
  if (grp) {
    brcm_dev = grp->brcm_devices_array[brcm_port];
    if(!brcm_dev)
      brcm_dev = grp->brcm_devices_array[BRCM_ANY_PORT];
    return brcm_dev;
  }
  
  return NULL;
}

static struct packet_type brcm_packet_type __read_mostly = {
  .type = cpu_to_be16(ETH_P_BRCM),
  .func = brcm_skb_recv, /* BRCM receive method */
};

static void brcm_rcu_free(struct rcu_head *rcu)
{
  brcm_group_free(container_of(rcu, struct brcm_group, rcu));
}

void unregister_brcm_dev(struct net_device *dev, struct list_head *head)
{
  struct brcm_dev_info *brcm = brcm_dev_info(dev);
  struct net_device *real_dev = brcm->real_dev;
  struct brcm_group *grp;
  u16 brcm_port = brcm->brcm_port;
  
  ASSERT_RTNL();
  
  grp = brcm_find_group(real_dev);
  BUG_ON(!grp);
  grp->nr_ports--;
  
  brcm_group_set_device(grp, brcm_port, NULL);
  if (!grp->killall)
    synchronize_net();
  
  unregister_netdevice_queue(dev, head);
  
  /* If the group is now empty, kill off the group. */
  if (grp->nr_ports == 0) {
    hlist_del_rcu(&grp->hlist);
  
    /* Free the group, after all cpu's are done. */
    call_rcu(&grp->rcu, brcm_rcu_free);
  }
  
  /* Get rid of the vlan's reference to real_dev */
  dev_put(real_dev);
}

static void brcm_sync_address(struct net_device *dev, struct net_device *brcmdev)
{
  struct brcm_dev_info *brcm = brcm_dev_info(brcmdev);
  
  /* May be called without an actual change */
  if (!compare_ether_addr(brcm->real_dev_addr, dev->dev_addr))
    return;
  
  /* vlan address was different from the old address and is equal to
   * the new address */
  if (compare_ether_addr(brcmdev->dev_addr, brcm->real_dev_addr) &&
      !compare_ether_addr(brcmdev->dev_addr, dev->dev_addr))
    dev_unicast_delete(dev, brcmdev->dev_addr);
  
  /* vlan address was equal to the old address and is different from
   * the new address */
  if (!compare_ether_addr(brcmdev->dev_addr, brcm->real_dev_addr) &&
      compare_ether_addr(brcmdev->dev_addr, dev->dev_addr))
    dev_unicast_add(dev, brcmdev->dev_addr);
 
  memcpy(brcm->real_dev_addr, dev->dev_addr, ETH_ALEN);
}

static int register_brcm_dev(struct net_device *real_dev, u16 brcm_port)
{
  struct net_device *new_dev;
  struct net *net = dev_net(real_dev);
  struct brcm_group *grp;
  char name[IFNAMSIZ];
  int err;
  
  if(brcm_port >= BRCM_PORT_MASK)
    return -ERANGE;
  
  // exist yet
  if (get_brcm_dev(real_dev, brcm_port) != NULL)
    return -EEXIST;
  
  snprintf(name, IFNAMSIZ, "brcm%i", brcm_port);
  new_dev = alloc_netdev_mq(sizeof(struct brcm_dev_info), name, brcm_setup, 1);
  if (new_dev == NULL)
    return -ENOBUFS;
  new_dev->real_num_tx_queues = real_dev->real_num_tx_queues;
  dev_net_set(new_dev, net);
  new_dev->mtu = real_dev->mtu;
  
  brcm_dev_info(new_dev)->brcm_port = brcm_port;
  brcm_dev_info(new_dev)->real_dev = real_dev;
  brcm_dev_info(new_dev)->dent = NULL;
  //new_dev->rtnl_link_ops = &brcm_link_ops;
  
  grp = brcm_find_group(real_dev);
  if (!grp)
    grp = brcm_group_alloc(real_dev);
  
  err = register_netdevice(new_dev);
  if (err < 0)
    goto out_free_newdev;
  
  /* Account for reference in struct brcm_dev_info */
  dev_hold(real_dev);
  brcm_group_set_device(grp, brcm_port, new_dev);
  grp->nr_ports++;
  
  return 0;
  
out_free_newdev:
  free_netdev(new_dev);
  return err;
}

static void __brcm_device_event(struct net_device *dev, unsigned long event)
{
  switch (event) {
  case NETDEV_CHANGENAME:
    brcm_proc_rem_dev(dev);
    if (brcm_proc_add_dev(dev) < 0)
      pr_warning("BRCM: failed to change proc name for %s\n", dev->name);
    break;
  case NETDEV_REGISTER:
    if (brcm_proc_add_dev(dev) < 0)
      pr_warning("BRCM: failed to add proc entry for %s\n", dev->name);
    break;
  case NETDEV_UNREGISTER:
    brcm_proc_rem_dev(dev);
    break;
  }
}


static int brcm_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
  struct net_device *dev = ptr;
  struct brcm_group *grp;
  int i, flgs;
  struct net_device *brcmdev;
  struct brcm_dev_info *brcm;
  LIST_HEAD(list);
  
  if (is_brcm_dev(dev))
    __brcm_device_event(dev, event);
  
  // only phy dev can through it
  grp = brcm_find_group(dev);
  if (!grp)
    goto out;
  
  switch (event) {
    case NETDEV_CHANGE:
    /* Propagate real device state to vlan devices */
      for (i = 0; i < BRCM_GROUP_ARRAY_LEN; i++) {
        brcmdev = brcm_group_get_device(grp, i);
        if (!brcmdev)
          continue;
  
        netif_stacked_transfer_operstate(dev, brcmdev);
      }
      break;
  
    case NETDEV_CHANGEADDR:
    /* Adjust unicast filters on underlying device */
      for (i = 0; i < BRCM_GROUP_ARRAY_LEN; i++) {
        brcmdev = brcm_group_get_device(grp, i);
        if (!brcmdev)
          continue;
  
        flgs = brcmdev->flags;
        if (!(flgs & IFF_UP))
          continue;
  
        brcm_sync_address(dev, brcmdev);
      }
      break;
  
    case NETDEV_CHANGEMTU:
      for (i = 0; i < BRCM_GROUP_ARRAY_LEN; i++) {
        brcmdev = brcm_group_get_device(grp, i);
        if (!brcmdev)
          continue;
  
      if (brcmdev->mtu <= dev->mtu)
        continue;
  
      dev_set_mtu(brcmdev, dev->mtu);
      }
      break;
  
    case NETDEV_DOWN:
      /* Put all VLANs for this dev in the down state too.  */
      for (i = 0; i < BRCM_GROUP_ARRAY_LEN; i++) {
        brcmdev = brcm_group_get_device(grp, i);
        if (!brcmdev)
          continue;
  
        flgs = brcmdev->flags;
        if (!(flgs & IFF_UP))
          continue;
  
        brcm = brcm_dev_info(brcmdev);
        dev_change_flags(brcmdev, flgs & ~IFF_UP);
        netif_stacked_transfer_operstate(dev, brcmdev);
      }
      break;
  
    case NETDEV_UP:
      /* Put all VLANs for this dev in the up state too.  */
      for (i = 0; i < BRCM_GROUP_ARRAY_LEN; i++) {
        brcmdev = brcm_group_get_device(grp, i);
        if (!brcmdev)
          continue;
  
      flgs = brcmdev->flags;
      if (flgs & IFF_UP)
        continue;
  
      brcm = brcm_dev_info(brcmdev);
      dev_change_flags(brcmdev, flgs | IFF_UP);
      netif_stacked_transfer_operstate(dev, brcmdev);
      }
      break;
  
    case NETDEV_UNREGISTER:
      /* Delete all BRCMs for this dev. */
      grp->killall = 1;
  
      for (i = 0; i < BRCM_GROUP_ARRAY_LEN; i++) {
        brcmdev = brcm_group_get_device(grp, i);
        if (!brcmdev)
          continue;
  
        /* unregistration of last brcm destroys group, abort afterwards */
        if (grp->nr_ports == 1)
          i = BRCM_GROUP_ARRAY_LEN;
  
        unregister_brcm_dev(brcmdev, &list);
      }
      unregister_netdevice_many(&list);
      break;
    }
  
out:
  return NOTIFY_DONE;
}


static struct notifier_block brcm_notifier_block __read_mostly = {
  .notifier_call = brcm_device_event,
};

/*
  *	BRCM IOCTL handler.
  *	o execute requested action or pass command to the device driver
  *   arg is really a struct brcm_ioctl_args __user *.
  */
static int brcm_ioctl_handler(struct net *net, void __user *arg)
{
  int err;
  struct brcm_ioctl_args args;
  struct net_device *dev = NULL;
  
  if (copy_from_user(&args, arg, sizeof(struct brcm_ioctl_args)))
  	return -EFAULT;
  
  /* Null terminate this sucker, just in case. */
  args.device1[23] = 0;
  args.u.device2[23] = 0;
  
  rtnl_lock();
  
  switch (args.cmd) {
  case ADD_BRCM_CMD:
  case DEL_BRCM_CMD:
    err = -ENODEV;
    dev = __dev_get_by_name(net, args.device1);
    if (!dev)
      goto out;
  
    err = -EINVAL;
    if (args.cmd != ADD_BRCM_CMD && !is_brcm_dev(dev))
      goto out;
  }
  
  switch (args.cmd) {
  case ADD_BRCM_CMD:
    err = -EPERM;
    if (!capable(CAP_NET_ADMIN))
      break;
    err = register_brcm_dev(dev, args.u.port);
    break;
  
  case DEL_BRCM_CMD:
    err = -EPERM;
    if (!capable(CAP_NET_ADMIN))
      break;
    unregister_brcm_dev(dev, NULL);
    err = 0;
    break;

  default:
    err = -EOPNOTSUPP;
    break;
  }
out:
  rtnl_unlock();
  return err;
}


static int __init brcm_proto_init(void)
{
  int err;

  pr_info("%s v%s %s\n", brcm_fullname, brcm_version, brcm_copyright);
  pr_info("All bugs added by %s\n", brcm_buggyright);

  // register namespace
  err = register_pernet_subsys(&brcm_net_ops);
  if (err < 0)
    goto err0;

  err = register_netdevice_notifier(&brcm_notifier_block);
  if (err < 0)
  	goto err2;
  
  //err = brcm_netlink_init();
  //if (err < 0)
  //	goto err3;
  
  dev_add_pack(&brcm_packet_type);
  brcm_ioctl_set(brcm_ioctl_handler);
  return 0;
  
err3:
  unregister_netdevice_notifier(&brcm_notifier_block);
err2:
  unregister_pernet_subsys(&brcm_net_ops);
err0:
  return err;
}

static void __exit brcm_cleanup_module(void)
{
  unsigned int i;

  brcm_ioctl_set(NULL);

  unregister_netdevice_notifier(&brcm_notifier_block);

  dev_remove_pack(&brcm_packet_type);

  /* This table must be empty if there are no module references left. */
  for (i = 0; i < BRCM_GRP_HASH_SIZE; i++)
    BUG_ON(!hlist_empty(&brcm_group_hash[i]));

  unregister_pernet_subsys(&brcm_net_ops);
  rcu_barrier(); /* Wait for completion of call_rcu()'s */
}

module_init(brcm_proto_init);
module_exit(brcm_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

