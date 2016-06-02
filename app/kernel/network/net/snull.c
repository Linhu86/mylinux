
/*
 * snull.c --  the Simple Network Utility
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: snull.c,v 1.21 2004/11/05 02:36:03 rubini Exp $
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/interrupt.h> /* mark_bh */

#include <linux/in.h>
#include <linux/netdevice.h>   /* struct device, and other headers */
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/ip.h>          /* struct iphdr */
#include <linux/tcp.h>         /* struct tcphdr */
#include <linux/skbuff.h>
#include <linux/ethtool.h>

#include "snull.h"

#include <linux/in6.h>
#include <asm/checksum.h>

MODULE_AUTHOR("Linhu Ying");
MODULE_LICENSE("Dual BSD/GPL");

static int lockup = 0;
module_param(lockup, int, 0);

static int timeout = SNULL_TIMEOUT;
module_param(timeout, int, 0);


static int use_napi = 0;
module_param(use_napi, int , 0);

struct snull_packet
{
  struct snull_packet *next;
  struct net_device *dev;
  int datalen;
  u8 data[ETH_DATA_LEN];
};

int pool_size = 8;
module_param(pool_size, int, 0);

struct snull_priv {
  struct net_device_stats stats;
  int status;
  struct snull_packet *ppool;
  struct snull_packet *rx_queue;
  int rx_int_enabled;
  int tx_packetlen;
  u8 *tx_packetdata;
  struct sk_buff *skb;
  spinlock_t lock;
  struct net_device *dev;
  struct napi_struct napi;
};

static void snull_tx_timeout(struct net_device *dev);

static void(*snull_interrupt)(int, void *, struct pt_regs *);

void snull_setup_pool(struct net_device *dev)
{
  struct snull_priv *priv = netdev_priv(dev);
  int i;
  struct snull_packet *pkt;

  priv->ppool = NULL;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);


  for(i = 0; i < pool_size; i++)
  {
    pkt = kmalloc(sizeof(struct snull_packet), GFP_KERNEL);
    if(pkt == NULL)
    {
      printk(KERN_NOTICE "Ran out of memory allocating packets.\n");
      return;
    }
    pkt->dev = dev;
    pkt->next = priv ->ppool;
    priv->ppool = pkt;
  }
}

void snull_teardown_pool(struct net_device *dev)
{
  struct snull_priv *priv = netdev_priv(dev);
  struct snull_packet *pkt;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);


  while((pkt = priv->ppool))
  {
    priv->ppool = pkt->next;
    kfree(pkt);
  }
}

struct snull_packet *snull_get_tx_buffer(struct net_device *dev)
{
  struct snull_priv *priv = netdev_priv(dev);
  unsigned long flags;
  struct snull_packet *pkt;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);


  spin_lock_irqsave(&priv->lock, flags);

  pkt = priv->ppool;

  priv->ppool = pkt->next;

  if(priv->ppool == NULL)
  {
     printk(KERN_INFO "Pool empty\n");
     netif_stop_queue(dev);
  }
  spin_unlock_irqrestore(&priv->lock, flags);
  return pkt;
}

void snull_release_buffer(struct snull_packet *pkt)
{
  unsigned long flags;
  struct snull_priv *priv = netdev_priv(pkt->dev);

  spin_lock_irqsave(&priv->lock, flags);

  pkt->next = priv->ppool;
  priv->ppool = pkt;

  spin_unlock_irqrestore(&priv->lock, flags);

  if(netif_queue_stopped(pkt->dev) && pkt->next == NULL)
  {
    netif_wake_queue(pkt->dev);
  }

}

void snull_enqueue_buf(struct net_device *dev, struct snull_packet *pkt)
{
  unsigned long flags;
  struct snull_priv *priv = netdev_priv(dev);

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);



  spin_lock_irqsave(&priv->lock, flags);
  pkt->next = priv->rx_queue;
  priv->rx_queue = pkt;
  spin_unlock_irqrestore(&priv->lock, flags);
}

struct snull_packet *snull_dequeue_buf(struct net_device *dev)
{
  struct snull_priv *priv = netdev_priv(dev);
  struct snull_packet *pkt;
  unsigned long flags;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  spin_lock_irqsave(&priv->lock, flags);
  pkt = priv->rx_queue;
  if(pkt != NULL)
  {
    priv->rx_queue = pkt->next;
  }
  spin_unlock_irqrestore(&priv->lock, flags);
  return pkt;
}

static void snull_rx_ints(struct net_device *dev, int enable)
{
  struct snull_priv *priv = netdev_priv(dev);
  
  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  priv->rx_int_enabled = enable;
}

int snull_open(struct net_device *dev)
{

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);


  memcpy(dev->dev_addr, "\0SNUL0", ETH_ALEN);
  if (dev == snull_devs[1])
    dev->dev_addr[ETH_ALEN-1]++; /* \0SNUL1 */
  netif_start_queue(dev);
  return 0;
}

int snull_release(struct net_device *dev)
{
    if(dev == snull_devs[0])
      printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
    else
       printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

    /* release ports, irq and such -- like fops->close */
  netif_stop_queue(dev); /* can't transmit any more */
  return 0;
}

int snull_config(struct net_device *dev, struct ifmap *map)
{
    if(dev == snull_devs[0])
      printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
    else
       printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);



  if(dev->flags & IFF_UP)
  {
    return -EBUSY;
  }

  if(map->base_addr != dev->base_addr)
  {
    printk(KERN_WARNING "snull: can not change I/O address.\n");
    return -EOPNOTSUPP;
  }

  if(map->irq != dev->irq)
  {
    dev->irq= map->irq;
  }

  return 0;
}

void snull_rx(struct net_device *dev, struct snull_packet *pkt)
{
  struct sk_buff *skb;
  struct snull_priv *priv = netdev_priv(dev);

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  skb = dev_alloc_skb(pkt->datalen + 2);

  if (!skb) {
      if (printk_ratelimit())
          printk(KERN_NOTICE "snull rx: low on mem - packet dropped\n");
      priv->stats.rx_dropped++;
      goto out;
  }

  skb_reserve(skb, 2);
  memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);

  skb->dev = dev;
  skb->protocol = eth_type_trans(skb, dev);
  skb->ip_summed = CHECKSUM_UNNECESSARY;
  priv->stats.rx_packets++;
  priv->stats.rx_bytes += pkt->datalen;

//   if (1) { /* enable this conditional to look at the data */
     int i = 0;
     struct iphdr *iph = (struct iphdr*) skb->data;
     char *buf = skb->data;
     unsigned int len = skb->len;
     printk("len is %i protocol is :%d\n" KERN_INFO "data:",len, iph->protocol);
     for (i=14 ; i<len; i++)
     {
        printk(" %02x",buf[i]&0xff);
     }
     printk("\n");
//  }

 if(iph->protocol == IPPROTO_ICMP)
 {
   printk(KERN_INFO "Received an ICMP packet.\n");
 }
  
  netif_rx(skb);

  out:
    return;
}

static int snull_poll(struct napi_struct *napi, int budget)
{
  int npackets = 0;
  struct sk_buff *skb;
  struct snull_priv *priv = container_of(napi, struct snull_priv, napi);
  struct net_device *dev = priv->dev;
  struct snull_packet *pkt;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  while(npackets < budget && priv->rx_queue)
  {
    pkt = snull_dequeue_buf(dev);
    skb = dev_alloc_skb(pkt->datalen + 2);

    if(!skb)
    {
       if(printk_ratelimit())
         printk(KERN_NOTICE "snull: packet dropped.\n");
       priv->stats.rx_dropped ++;
       snull_release_buffer(pkt);
       continue;
    }

    skb_reserve(skb, 2);
    memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);
    skb->dev = dev;
    skb->protocol = eth_type_trans(skb, dev);
    skb->ip_summed = CHECKSUM_UNNECESSARY;
    netif_receive_skb(skb);
  }

  if(!priv->rx_queue)
  {
    napi_complete(napi);
    snull_rx_ints(dev, 1);
    return 0;
  }

  return npackets;
}

static void snull_regular_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
  int statusword;
  struct snull_priv *priv;
  struct snull_packet *pkt = NULL;
  struct net_device *dev = (struct net_device *)dev_id;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  if(!dev)
  {
    return;
  }

  priv = netdev_priv(dev);

  spin_lock(&priv->lock);

  statusword = priv->status;
  priv->status = 0;

  if(statusword & SNULL_RX_INTR)
  {
    pkt = priv->rx_queue;
    if(pkt)
    {
       priv->rx_queue = pkt->next;

 //      printk(KERN_INFO "packet data payload: %s", pkt->data);
       
       snull_rx(dev, pkt);
    }
  }

  if(statusword & SNULL_TX_INTR)
  {
    priv->stats.tx_packets ++;
    priv->stats.tx_bytes += priv->tx_packetlen;
    dev_kfree_skb(priv->skb);
  }

  spin_unlock(&priv->lock);

  if(pkt)
  {
    snull_release_buffer(pkt);
  }
  return;
}

static void snull_napi_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
  int statusword;
  struct snull_priv *priv;
  struct net_device *dev = (struct net_device *)dev_id;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  if(!dev)
  {
    return;
  }

  priv = netdev_priv(dev);

  spin_lock(&priv->lock);

  statusword = priv->status;

  priv->status = 0;

  if(statusword & SNULL_RX_INTR)
  {
    snull_rx_ints(dev, 0);
    napi_schedule(&priv->napi);
  }

  if(statusword & SNULL_TX_INTR)
  {
    priv->stats.tx_packets ++;
    priv->stats.tx_bytes += priv->tx_packetlen;
    dev_kfree_skb(priv->skb);
  }

  spin_unlock(&priv->lock);
  return;
}

static void snull_hw_tx(char *buf, int len, struct net_device *dev)
{
  struct iphdr *ih;
  struct net_device *dest;
  struct snull_priv *priv;

  u32 *saddr, *daddr;

  struct snull_packet *tx_buffer;
  
  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  if(len < sizeof(struct ethhdr) + sizeof(struct iphdr))
  {
      printk(KERN_INFO "snull: Hmm... packet too short (%i octets)\n",len);
      return;
  }

   if (1) { /* enable this conditional to look at the data */
     int i;
     printk("len is %i\n" KERN_INFO "data:",len);
     for (i=14 ; i<len; i++)
     {
        printk(" %02x",buf[i]&0xff);
     }
     printk("\n");
  }

  ih = (struct iphdr *)(buf + sizeof(struct ethhdr));
  saddr = &ih->saddr;
  daddr = &ih->daddr;

  ((u8 *)saddr)[2] ^= 1;
  ((u8 *)daddr)[2] ^= 1;

  ih->check = 0;
  ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);

  if (dev == snull_devs[0])
  {
     printk(KERN_INFO "%08x:%05i --> %08x:%05i\n",
     ntohl(ih->saddr),ntohs(((struct tcphdr *)(ih+1))->source),
     ntohl(ih->daddr),ntohs(((struct tcphdr *)(ih+1))->dest));
  }
  else
  {
     printk(KERN_INFO "%08x:%05i <-- %08x:%05i\n",
     ntohl(ih->daddr),ntohs(((struct tcphdr *)(ih+1))->dest),
     ntohl(ih->saddr),ntohs(((struct tcphdr *)(ih+1))->source));
  }

  dest = snull_devs[dev == snull_devs[0] ? 1:0];
  priv  = netdev_priv(dest);
  tx_buffer = snull_get_tx_buffer(dev);
  tx_buffer->datalen = len;
  memcpy(tx_buffer->data, buf, len);
  snull_enqueue_buf(dest, tx_buffer);

  if(priv->rx_int_enabled)
  {
    priv->status |= SNULL_RX_INTR;
    snull_interrupt(0, dest, NULL);
  }

  priv = netdev_priv(dev);
  priv->tx_packetlen = len;
  priv->tx_packetdata = buf;
  priv->status |= SNULL_TX_INTR;
  
  if (lockup && ((priv->stats.tx_packets + 1) % lockup) == 0) {
          /* Simulate a dropped transmit interrupt */
    netif_stop_queue(dev);
    printk(KERN_INFO "Simulate lockup at %ld, txp %ld\n", jiffies,
              (unsigned long) priv->stats.tx_packets);
  }
  else
  {
      snull_interrupt(0, dev, NULL);
  }
}

int snull_tx(struct sk_buff *skb, struct net_device *dev)
{
  int len;
  char *data, shortpkt[ETH_ZLEN];
  struct snull_priv *priv = netdev_priv(dev);

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  data = skb->data;
  len = skb->len;

  if(len < ETH_ZLEN)
  {
    memset(shortpkt, 0, ETH_ZLEN);
    memcpy(shortpkt, skb->data, skb->len);
    len = ETH_ZLEN;
    data = shortpkt;
  }

  dev->trans_start = jiffies;

  priv->skb = skb;

  snull_hw_tx(data, len, dev);

  return 0;
}

void snull_tx_timeout(struct net_device *dev)
{
  struct snull_priv *priv = netdev_priv(dev);

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  PDEBUG("Transmit timeout at %ld, latency %ld\n", jiffies, jiffies - dev->trans_start);

  priv->status = SNULL_TX_INTR;

  snull_interrupt(0, dev, NULL);

  priv->stats.tx_errors ++;

  netif_wake_queue(dev);

  return;
}

int snull_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    if(dev == snull_devs[0])
      printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
    else
       printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  return 0;
}

struct net_device_stats *snull_stats(struct net_device *dev)
{
  struct snull_priv *priv = netdev_priv(dev);
  return &priv->stats;
}

int snull_rebuild_header(struct sk_buff *skb)
{
  struct ethhdr *eth = (struct ethhdr *)skb->data;
  struct net_device *dev = skb->dev;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);



  memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
  memcpy(eth->h_dest, dev->dev_addr, dev->addr_len);

  eth->h_dest[ETH_ALEN-1]   ^= 0x01;   /* dest is us xor 1 */
  return 0;
}

int snull_header(struct sk_buff *skb, struct net_device *dev,
                          unsigned short type, const void *daddr, const void *saddr,
                          unsigned len)
{
  struct ethhdr *eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);


  eth->h_proto = htons(type);
  memcpy(eth->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
  memcpy(eth->h_dest,   daddr ? daddr : dev->dev_addr, dev->addr_len);
  eth->h_dest[ETH_ALEN-1]   ^= 0x01;   /* dest is us xor 1 */
  return (dev->hard_header_len);
}

int snull_change_mtu(struct net_device *dev, int new_mtu)
{
  unsigned long flags;
  struct snull_priv *priv = netdev_priv(dev);
  spinlock_t *lock = &priv->lock;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);



  if((new_mtu < 68) || (new_mtu > 1500))
  {
    printk(KERN_ERR "Invalid MTU value.\n");
    return -EINVAL;
  }
  else
  {
     printk(KERN_INFO "Change MTU to %d.\n", new_mtu);
  }

  spin_lock_irqsave(lock, flags);
  dev->mtu = new_mtu;
  spin_unlock_irqrestore(lock, flags);
  return 0;
}

static const struct header_ops snull_header_ops = 
{
  .create = snull_header,
  .rebuild = snull_rebuild_header
};

static int snull_set_settings(struct net_device *netdev, struct ethtool_cmd *cmd)
{
    if(netdev == snull_devs[0])
      printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
    else
       printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);

  printk("ioctl operation.\n");

  return 1;
}


static const struct net_device_ops snull_netdev_ops = 
{
  .ndo_open            = snull_open,
  .ndo_stop             = snull_release,
  .ndo_start_xmit    = snull_tx,
  .ndo_do_ioctl        = snull_ioctl,
  .ndo_set_config     = snull_config,
  .ndo_get_stats      = snull_stats,
  .ndo_change_mtu  = snull_change_mtu,
  .ndo_tx_timeout    = snull_tx_timeout
};

static const struct ethtool_ops e100_ethtool_ops = {
  .set_settings = snull_set_settings
};


void snull_init(struct net_device *dev)
{
  struct snull_priv *priv;

  if(dev == snull_devs[0])
    printk(KERN_INFO "[Device 0] Entry: %s\n", __FUNCTION__);
  else
     printk(KERN_INFO "[Device 1] Entry: %s\n", __FUNCTION__);


  ether_setup(dev);

  dev->watchdog_timeo = timeout;
  dev->netdev_ops = &snull_netdev_ops;
  dev->header_ops = &snull_header_ops;

  dev->flags |= IFF_NOARP;
  dev->features |= NETIF_F_HW_CSUM;

  priv = netdev_priv(dev);

  if(use_napi)
  {
    netif_napi_add(dev, &priv->napi, snull_poll, 2);
  }
  memset(priv, 0, sizeof(struct snull_priv));
  spin_lock_init(&priv->lock);
  snull_rx_ints(dev, 1);
  snull_setup_pool(dev);
}

struct net_device *snull_devs[2];

void snull_cleanup(void)
{
  int i;

  for(i = 0; i < 2; i++)
  {
    if(snull_devs[i])
    {
      unregister_netdev(snull_devs[i]);
      snull_teardown_pool(snull_devs[i]);
      free_netdev(snull_devs[i]);
      printk(KERN_INFO "snull: deregistering device \"%s\"\n", snull_devs[i]->name);
    }
  }
  return;
}

int snull_init_module(void)
{
  int result, i, ret = -ENOMEM;

  snull_interrupt = use_napi ? snull_napi_interrupt : snull_regular_interrupt;

  snull_devs[0] = alloc_netdev(sizeof(struct snull_priv), "sn%d", snull_init);

  snull_devs[1] = alloc_netdev(sizeof(struct snull_priv), "sn%d", snull_init);

  if(snull_devs[0] == NULL || snull_devs[1] == NULL)
    goto out;

  ret = -ENODEV;

  for(i = 0; i < 2; i++)
  {
    if(result = register_netdev(snull_devs[i]))
    {
      printk("snull: error %i registering device \"%s\"\n",
      result, snull_devs[i]->name);
    }
    else
    {
      printk(KERN_INFO "snull: registering device \"%s\"\n", snull_devs[i]->name);
      ret = 0;
    }
    snull_devs[i]->ethtool_ops = &e100_ethtool_ops;
  }
  
  out:
    if (ret) 
      snull_cleanup();
    return ret;
  }
  
 module_init(snull_init_module);
 module_exit(snull_cleanup);





