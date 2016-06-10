#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>

#include "brcm.h"

static inline struct sk_buff *brcm_check_reorder_header(struct sk_buff *skb)
{
  if (skb_cow(skb, skb_headroom(skb)) < 0)
    skb = NULL;
  if (skb) {
    /* Lifted from Gleb's VLAN code... */
    memmove(skb->data - ETH_HLEN,
      skb->data - BRCM_ETH_HLEN, 12);
    skb->mac_header += BRCM_HLEN;
  }

  return skb;
}

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
  if (op != BRCM_RCV_OP || brcm_port < 1 
    || brcm_port > 27) 
    goto err_unlock;

  skb->dev = find_brcm_dev(dev, brcm_port);
  if (!skb->dev) {
    goto err_unlock;
  }

  rx_stats = per_cpu_ptr(brcm_dev_info(skb->dev)->brcm_rx_stats,
             smp_processor_id());
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

static int brcm_dev_change_mtu(struct net_device *dev, int new_mtu)
{
  /* TODO: gotta make sure the underlying layer can handle it,
   * maybe an IFF_BRCM_CAPABLE flag for devices?
   */
  if (brcm_dev_info(dev)->real_dev->mtu < new_mtu)
    return -ERANGE;

  dev->mtu = new_mtu;
  return 0;
}

static int brcm_dev_init(struct net_device *dev)
{
  struct net_device *real_dev = brcm_dev_info(dev)->real_dev;

  netif_carrier_off(dev);

#if 0
  /* IFF_BROADCAST|IFF_MULTICAST; ??? */
  dev->flags  = real_dev->flags;
  dev->iflink = real_dev->ifindex;
  dev->state  = real_dev->state;

  //dev->features = real_dev->features;
  dev->gso_max_size = real_dev->gso_max_size;

  /* ipv6 shared card related stuff */
  dev->dev_id = real_dev->dev_id;
#endif

  if (is_zero_ether_addr(dev->dev_addr))
    memcpy(dev->dev_addr, real_dev->dev_addr, dev->addr_len);
  if (is_zero_ether_addr(dev->broadcast))
    memcpy(dev->broadcast, real_dev->broadcast, dev->addr_len);

  brcm_dev_info(dev)->brcm_rx_stats = alloc_percpu(struct brcm_rx_stats);
  if (!brcm_dev_info(dev)->brcm_rx_stats)
    return -ENOMEM;

  return 0;
}

static void brcm_dev_uninit(struct net_device *dev)
{
  struct brcm_dev_info *brcm = brcm_dev_info(dev);

  free_percpu(brcm->brcm_rx_stats);
  brcm->brcm_rx_stats = NULL;
}

static int brcm_dev_open(struct net_device *dev)
{
  struct brcm_dev_info *brcm = brcm_dev_info(dev);
  struct net_device *real_dev = brcm->real_dev;
  int err;
  
  if (!(real_dev->flags & IFF_UP))
    return -ENETDOWN;
  
  if (compare_ether_addr(dev->dev_addr, real_dev->dev_addr)) {
    err = dev_unicast_add(real_dev, dev->dev_addr);
    if (err < 0)
      goto out;
  }

  if (dev->flags & IFF_ALLMULTI) {
    err = dev_set_allmulti(real_dev, 1);
    if (err < 0)
      goto del_unicast;
  }
  if (dev->flags & IFF_PROMISC) {
    err = dev_set_promiscuity(real_dev, 1);
    if (err < 0)
      goto clear_allmulti;
  }

  memcpy(brcm->real_dev_addr, real_dev->dev_addr, ETH_ALEN);
  netif_carrier_on(dev);
  return 0;

clear_allmulti:
  if (dev->flags & IFF_ALLMULTI)
    dev_set_allmulti(real_dev, -1);
del_unicast:
  if (compare_ether_addr(dev->dev_addr, real_dev->dev_addr))
    dev_unicast_delete(real_dev, dev->dev_addr);
out:
  netif_carrier_off(dev);
  return err;
}

static int brcm_dev_stop(struct net_device *dev)
{
  struct brcm_dev_info *brcm = brcm_dev_info(dev);
  struct net_device *real_dev = brcm->real_dev;

  dev_mc_unsync(real_dev, dev);
  dev_unicast_unsync(real_dev, dev);
  if (dev->flags & IFF_ALLMULTI)
    dev_set_allmulti(real_dev, -1);
  if (dev->flags & IFF_PROMISC)
    dev_set_promiscuity(real_dev, -1);

  if (compare_ether_addr(dev->dev_addr, real_dev->dev_addr))
    dev_unicast_delete(real_dev, dev->dev_addr);

  netif_carrier_off(dev);
  return 0;
}

static int brcm_dev_set_mac_address(struct net_device *dev, void *p)
{
  struct net_device *real_dev = brcm_dev_info(dev)->real_dev;
  struct sockaddr *addr = p;
  int err;

  if (!is_valid_ether_addr(addr->sa_data))
    return -EADDRNOTAVAIL;

  if (!(dev->flags & IFF_UP))
    goto out;

  if (compare_ether_addr(addr->sa_data, real_dev->dev_addr)) {
    err = dev_unicast_add(real_dev, addr->sa_data);
    if (err < 0)
      return err;
  }

  if (compare_ether_addr(dev->dev_addr, real_dev->dev_addr))
    dev_unicast_delete(real_dev, dev->dev_addr);

out:
  memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
  return 0;
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

    //if (orig_headroom < BRCM_HLEN)
    //  brcm_dev_info(dev)->cnt_inc_headroom_on_tx++;
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

static struct net_device_stats *brcm_dev_get_stats(struct net_device *dev)
{
  struct net_device_stats *stats = &dev->stats;

  dev_txq_stats_fold(dev, stats);

  if (brcm_dev_info(dev)->brcm_rx_stats) {
    struct brcm_rx_stats *p, rx = {0};
    int i;

    for_each_possible_cpu(i) {
      p = per_cpu_ptr(brcm_dev_info(dev)->brcm_rx_stats, i);
      rx.rx_packets += p->rx_packets;
      rx.rx_bytes   += p->rx_bytes;
      rx.rx_errors  += p->rx_errors;
      rx.multicast  += p->multicast;
    }
    stats->rx_packets = rx.rx_packets;
    stats->rx_bytes   = rx.rx_bytes;
    stats->rx_errors  = rx.rx_errors;
    stats->multicast  = rx.multicast;
  }
  return stats;
}

static void brcm_dev_set_rx_mode(struct net_device *brcm_dev)
{
  dev_mc_sync(brcm_dev_info(brcm_dev)->real_dev, brcm_dev);
  dev_unicast_sync(brcm_dev_info(brcm_dev)->real_dev, brcm_dev);
}

static void brcm_dev_change_rx_flags(struct net_device *dev, int change)
{
  struct net_device *real_dev = brcm_dev_info(dev)->real_dev;

  if (change & IFF_ALLMULTI)
    dev_set_allmulti(real_dev, dev->flags & IFF_ALLMULTI ? 1 : -1);
  if (change & IFF_PROMISC)
    dev_set_promiscuity(real_dev, dev->flags & IFF_PROMISC ? 1 : -1);
}

static int brcm_dev_neigh_setup(struct net_device *dev, struct neigh_parms *pa)
{
  struct net_device *real_dev = brcm_dev_info(dev)->real_dev;
  const struct net_device_ops *ops = real_dev->netdev_ops;
  int err = 0;

  if (netif_device_present(real_dev) && ops->ndo_neigh_setup)
    err = ops->ndo_neigh_setup(real_dev, pa);

  return err;
}


static const struct net_device_ops brcm_netdev_ops = {
  .ndo_change_mtu    = brcm_dev_change_mtu,
  .ndo_init    = brcm_dev_init,
  .ndo_uninit    = brcm_dev_uninit,
  .ndo_open    = brcm_dev_open,
  .ndo_stop    = brcm_dev_stop,
  .ndo_start_xmit =  brcm_dev_hard_start_xmit,
  .ndo_validate_addr  = eth_validate_addr,
  .ndo_set_mac_address  = brcm_dev_set_mac_address,
  .ndo_set_rx_mode  = brcm_dev_set_rx_mode,
  .ndo_set_multicast_list  = brcm_dev_set_rx_mode,
  .ndo_change_rx_flags  = brcm_dev_change_rx_flags,
  //.ndo_do_ioctl    = brcm_dev_ioctl,
  .ndo_neigh_setup  = brcm_dev_neigh_setup,
  .ndo_get_stats    = brcm_dev_get_stats,
};

static const struct ethtool_ops brcm_ethtool_ops = {
  //.get_settings          = vlan_ethtool_get_settings,
  //.get_drvinfo          = vlan_ethtool_get_drvinfo,
  //.get_link    = ethtool_op_get_link,
  //.get_rx_csum    = vlan_ethtool_get_rx_csum,
  //.get_flags    = vlan_ethtool_get_flags,
};

void brcm_setup(struct net_device *dev)
{
  ether_setup(dev);

  dev->priv_flags    |= IFF_BRCM_TAG;
  dev->priv_flags    &= ~IFF_XMIT_DST_RELEASE;
  dev->tx_queue_len  = 0;

  dev->netdev_ops    = &brcm_netdev_ops;
  dev->destructor    = free_netdev;
  dev->ethtool_ops  = &brcm_ethtool_ops;

  memset(dev->broadcast, 0, ETH_ALEN);
}

