#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fcntl.h>
#include <linux/aio.h>
#include <asm/uaccess.h>

static struct hlist_head brcm_group_hash[BRCM_GRP_HASH_SIZE];  
struct brcm_group {  
  struct hlist_node hlist;  
  struct net_device *real_dev;  
  int nr_ports;  
  int killall;  
  struct net_device *brcm_devices_array[BRCM_GROUP_ARRAY_LEN];  
  struct rcu_head  rcu;  
};

struct brcm_net {  
   /* /proc/net/brcm */  
  struct proc_dir_entry *proc_brcm_dir;  
  /* /proc/net/brcm/config */  
  struct proc_dir_entry *proc_brcm_conf;  
};  

err = register_pernet_subsys(&brcm_net_ops);  
static struct pernet_operations brcm_net_ops = {  
  .init = brcm_init_net,  
  .exit = brcm_exit_net,  
  .id = &brcm_net_id,  
  .size = sizeof(struct brcm_net),  
};  

err = register_netdevice_notifier(&brcm_notifier_block);  

static struct notifier_block brcm_notifier_block __read_mostly = {  
  .notifier_call = brcm_device_event,  
};  

struct net_device *find_brcm_dev(struct net_device *real_dev, u16 brcm_port)  
{  
  struct brcm_group *grp = brcm_find_group(real_dev);  
  if (grp)   
    brcm_dev = grp->brcm_devices_array[brcm_port];  
  return NULL;  
}

struct brcm_dev_info{  
  struct net_device  *real_dev;
  u16 brcm_port;  
  unsigned char  real_dev_addr[ETH_ALEN];  
  struct proc_dir_entry *dent;  
  struct brcm_rx_stats __percpu  *brcm_rx_stats;  
};

struct brcm_rx_stats {  
  unsigned long rx_packets;  
  unsigned long rx_bytes;  
  unsigned long multicast;  
  unsigned long rx_errors;  
};

static struct packet_type brcm_packet_type __read_mostly = {  
  .type = cpu_to_be16(ETH_P_BRCM),  
  .func = brcm_skb_recv, /* BRCM receive method */  
}; 

int brcm_skb_recv(struct sk_buff *skb, struct net_device *dev,  
    struct packet_type *ptype, struct net_device *orig_dev)  
{  
  struct brcm_hdr *bhdr;  
  struct brcm_rx_stats *rx_stats;  
  int op, brcm_port;  
   
  skb = skb_share_check(skb, GFP_ATOMIC);  
  if(!skb)  
    goto err_free;  
  bhdr = (struct brcm_hdr *)skb->data;  
  op = bhdr->brcm_tag.brcm_53242_op;  
  brcm_port = bhdr->brcm_tag.brcm_53242_src_portid- 23;  
   
  rcu_read_lock();  
   
  // drop wrong brcm tag packet  
  if (op != BRCM_RCV_OP || brcm_port < 1 || brcm_port > 27)   
    goto err_unlock;  
   
  skb->dev = find_brcm_dev(dev, brcm_port);  

  if (!skb->dev) {  
    goto err_unlock;  
  }  
   
  rx_stats = per_cpu_ptr(brcm_dev_info(skb->dev)->brcm_rx_stats, smp_processor_id());  
  rx_stats->rx_packets++;
  rx_stats->rx_bytes += skb->len; 
  skb_pull_rcsum(skb, BRCM_HLEN);
   
  switch (skb->pkt_type) {  
  case PACKET_BROADCAST: /* Yeah, stats collect these together.. */  
    /* stats->broadcast ++; // no such counter :-( */  
    break;  
   
  case PACKET_MULTICAST:  
    rx_stats->multicast++;  
    break;  
   
  case PACKET_OTHERHOST:  
    /* Our lower layer thinks this is not local, let's make sure. 
     * This allows the VLAN to have a different MAC than the 
     * underlying device, and still route correctly. 
     */  
    if (!compare_ether_addr(eth_hdr(skb)->h_dest,  
      skb->dev->dev_addr))  
    skb->pkt_type = PACKET_HOST;  
    break;  
  default:  
    break;  
  }  
    
  // set protocol  
  skb->protocol = bhdr->brcm_encapsulated_proto;  
   
  // reorder skb  
  skb = brcm_check_reorder_header(skb);  
  if (!skb) {  
    rx_stats->rx_errors++;  
    goto err_unlock;  
  }  
    
  netif_rx(skb);
  rcu_read_unlock();
  return NET_RX_SUCCESS;  
  
err_unlock:  
  rcu_read_unlock();  
  
err_free:  
  kfree_skb(skb);  
  return NET_RX_DROP;  
}  

static netdev_tx_t brcm_dev_hard_start_xmit(struct sk_buff *skb,  
         struct net_device *dev)  
{  
  int i = skb_get_queue_mapping(skb);  
  struct netdev_queue *txq = netdev_get_tx_queue(dev, i);  
  struct brcm_ethhdr *beth = (struct brcm_ethhdr *)(skb->data);  
  unsigned int len;  
  u16 brcm_port;  
  int ret;  
   
  /* Handle non-VLAN frames if they are sent to us, for example by DHCP. 
   * 
   * NOTE: THIS ASSUMES DIX ETHERNET, SPECIFICALLY NOT SUPPORTING 
   * OTHER THINGS LIKE FDDI/TokenRing/802.3 SNAPs... 
   */  
  if (beth->h_brcm_proto != htons(ETH_P_BRCM)){  
    //unsigned int orig_headroom = skb_headroom(skb);  
    brcm_t brcm_tag;  
    brcm_port = brcm_dev_info(dev)->brcm_port;  
    if (brcm_port == BRCM_ANY_PORT) {  
      brcm_tag.brcm_op_53242 = 0;  
      brcm_tag.brcm_tq_53242 = 0;  
      brcm_tag.brcm_te_53242 = 0;  
      brcm_tag.brcm_dst_53242 = 0;  
    }else {  
      brcm_tag.brcm_op_53242 = BRCM_SND_OP;  
      brcm_tag.brcm_tq_53242 = 0;  
      brcm_tag.brcm_te_53242 = 0;  
      brcm_tag.brcm_dst_53242 = brcm_port + 23;  
    }  
    
    skb = brcm_put_tag(skb, *(u32 *)(&brcm_tag));  
    if (!skb) {  
      txq->tx_dropped++;  
      return NETDEV_TX_OK;  
    }  
  }  
   
  skb_set_dev(skb, brcm_dev_info(dev)->real_dev);  
  len = skb->len;  
  ret = dev_queue_xmit(skb);  
   
  if (likely(ret == NET_XMIT_SUCCESS)) {  
    txq->tx_packets++;  
    txq->tx_bytes += len;  
  } else  
    txq->tx_dropped++;  
   
  return ret;  
}

void brcm_setup(struct net_device *dev)  
{  
  ether_setup(dev);  
   
  dev->priv_flags  |= IFF_BRCM_TAG;  
  dev->priv_flags  &= ~IFF_XMIT_DST_RELEASE;  
  dev->tx_queue_len = 0;  
   
  dev->netdev_ops  = &brcm_netdev_ops;  
  dev->destructor  = free_netdev;  
  dev->ethtool_ops = &brcm_ethtool_ops;  
   
  memset(dev->broadcast, 0, ETH_ALEN);  
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
  if (find_brcm_dev(real_dev, brcm_port) != NULL)  
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
    
  /* Account for reference in struct vlan_dev_info */  
  dev_hold(real_dev);  

  brcm_group_set_device(grp, brcm_port, new_dev);

  return 0;
   
out_free_newdev:  
  free_netdev(new_dev);  
  return err;  
}  

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



static const struct net_device_ops brcm_netdev_ops = {  
  .ndo_change_mtu         = brcm_dev_change_mtu,  
  .ndo_init               = brcm_dev_init,  
  .ndo_uninit             = brcm_dev_uninit,  
  .ndo_open               = brcm_dev_open,  
  .ndo_stop               = brcm_dev_stop,  
  .ndo_start_xmit         =  brcm_dev_hard_start_xmit,  
  .ndo_validate_addr      = eth_validate_addr,  
  .ndo_set_mac_address    = brcm_dev_set_mac_address,  
  .ndo_set_rx_mode        = brcm_dev_set_rx_mode,  
  .ndo_set_multicast_list = brcm_dev_set_rx_mode,  
  .ndo_change_rx_flags    = brcm_dev_change_rx_flags,  
  //.ndo_do_ioctl         = brcm_dev_ioctl,  
  .ndo_neigh_setup        = brcm_dev_neigh_setup,  
  .ndo_get_stats          = brcm_dev_get_stats,  
};  


static int __init brcm_proto_init(void)
{
  dev_add_pack(&brcm_packet_type);
}



