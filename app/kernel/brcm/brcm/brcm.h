#ifndef _LINUX_BRCM_H_
#define _LINUX_BRCM_H_

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/types.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <linux/skbuff.h>


#define BRCM_HLEN 6
#define BRCM_ETH_ALEN 6
#define BRCM_ETH_HLEN 20
#define BRCM_ANY_PORT 0

#define BRCM_PORT_MASK  0x1A /* BRCM Identifier */
#define BRCM_GRP_HASH_SHIFT 5
#define BRCM_GRP_HASH_SIZE (1 << BRCM_GRP_HASH_SHIFT)
#define BRCM_GRP_HASH_MASK (BRCM_GRP_HASH_SIZE - 1)
#define BRCM_GROUP_ARRAY_LEN 27

#define BRCM_RCV_OP 0x0
#define BRCM_SND_OP 0x2

typedef union {
  u32 brcm_val; // Raw 32-bit value
  
  struct {
    u32  _op:3,         /* opcode of this frame */
         _tq:2,         /* traffic queue */
         _te:2,         /* tag enforement */
         dst_pbmp:25;   /* destination pbmp */
  } brcm_tq_te_bmp;
#define     brcm_dst_53242  brcm_tq_te_bmp._dst_pbmp
#define     brcm_te_53242   brcm_tq_te_bmp._te
#define     brcm_tq_53242   brcm_tq_te_bmp._tq
#define     brcm_op_53242   brcm_tq_te_bmp._op

/* for bcm53242 IMP egress packet transfer*/
struct {
  u32 _op:3,      /* opcode of this frame */
      _cnt:12,    /* frame octet count */
      _cos:2,     /* COS queue */
          :1,     /* reserved */
      _reason:6, /* reason code */
      _ermon:2,
      _src_portid:6; /* source port id */
} brcm_53242_imp_egress_tag;


#define    brcm_53242_op            brcm_53242_imp_egress_tag._op
#define    brcm_53242_reason        brcm_53242_imp_egress_tag._reason
#define    brcm_53242_cosq          brcm_53242_imp_egress_tag._cos
#define    brcm_53242_src_portid    brcm_53242_imp_egress_tag._src_portid
} brcm_t;

struct brcm_hdr
{
  brcm_t brcm_tag;
  __be16 brcm_encapsulated_proto;
};

/**
 * struct brcm_ethhdr - brcm ethernet header (ethhdr + brcm_hdr)
 * @h_dest: destination ethernet address
 * @h_source: source ethernet address
 * @h_brcm_proto: ethernet protocol (always 0x8874)
 * @h_brcm_tq_te_bmp: brcm tag
 * @h_brcm_encapsulated_proto: packet type ID or len
 */
struct brcm_ethhdr {
  unsigned char h_dest[ETH_ALEN];
  unsigned char h_source[ETH_ALEN];
  __be16  h_brcm_proto;
  u8  h_brcm_tq_te_bmp[4];
  __be16  h_brcm_encapsulated_proto;
};

/* Passed in brcm_ioctl_args structure to determine behaviour. */
enum brcm_ioctl_cmds {
  ADD_BRCM_CMD,
  DEL_BRCM_CMD,
};

struct brcm_ioctl_args {
  int cmd; /* Should be one of the brcm_ioctl_cmds enum above. */
  char device1[24];

  union {
   char device2[24];
   int port;
   unsigned int skb_priority;
   unsigned int name_type;
   unsigned int bind_type;
   unsigned int flag; /* Matches brcm_dev_info flags */
  } u; 
};

struct brcm_rx_stats {
  unsigned long rx_packets;
  unsigned long rx_bytes;
  unsigned long multicast;
  unsigned long rx_errors;
};

struct brcm_dev_info{
  struct net_device  *real_dev;
  u16 brcm_port;
  unsigned char  real_dev_addr[ETH_ALEN];
  struct proc_dir_entry *dent;
  struct brcm_rx_stats __percpu  *brcm_rx_stats;
};

struct brcm_group {
  struct hlist_node hlist;
  struct net_device *real_dev;
  int nr_ports;
  int killall;
  struct net_device *brcm_devices_array[BRCM_GROUP_ARRAY_LEN];
  struct rcu_head  rcu;
};

struct proc_dir_entry;
extern int brcm_net_id;

struct brcm_net {
  /* /proc/net/brcm */
  struct proc_dir_entry *proc_brcm_dir;
  /* /proc/net/brcm/config */
  struct proc_dir_entry *proc_brcm_conf;
};

/* found in socket.c */
extern void brcm_ioctl_set(int (*hook)(struct net *, void __user *));

void brcm_setup(struct net_device *dev);

struct net_device *get_brcm_dev(struct net_device *real_dev, u16 brcm_port);
struct net_device *find_brcm_dev(struct net_device *real_dev, u16 brcm_port);

/* found in brcm_dev.c */
int brcm_skb_recv(struct sk_buff *skb, struct net_device *dev,
    struct packet_type *ptype, struct net_device *orig_dev);
void brcm_setup(struct net_device *dev);
void unregister_brcm_dev(struct net_device *dev, struct list_head *head);



static inline int is_brcm_dev(struct net_device *dev)
{
  return dev->priv_flags & IFF_BRCM_TAG;
}

static inline struct brcm_dev_info *brcm_dev_info(const struct net_device *dev)
{
  return netdev_priv(dev);
}

static inline struct sk_buff *brcm_put_tag(struct sk_buff *skb, u32 brcm_tq_te_bmp)
{
  struct brcm_ethhdr *beth;
 
  if (skb_cow_head(skb, BRCM_HLEN) < 0) {
   kfree_skb(skb);
   return NULL;
  }
  beth = (struct brcm_ethhdr *)skb_push(skb, BRCM_HLEN);
 
  /* Move the mac addresses to the beginning of the new header. */
  memmove(skb->data, skb->data + BRCM_HLEN, 2 * BRCM_ETH_ALEN);
  skb->mac_header -= BRCM_HLEN;
 
  /* first, the ethernet type */
  beth->h_brcm_proto = htons(ETH_P_BRCM);
 
  /* now, the TCI */
  *((u32 *)(beth->h_brcm_tq_te_bmp)) = htonl(brcm_tq_te_bmp);
  skb->protocol = htons(ETH_P_BRCM);
 
  // add 4 byte tag
 #define ETHERSMALL 60
  skb->len = ETHERSMALL + 10 > skb->len + 4 ? ETHERSMALL + 10 : skb->len + 4;
 
  return skb;
}


#endif

