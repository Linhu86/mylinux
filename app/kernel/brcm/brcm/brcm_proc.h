#ifndef __BRCM_PROC_INC__
#define __BRCM_PROC_INC__

#include <linux/netdevice.h>
//#ifdef CONFIG_PROC_FS
struct net;

int brcm_proc_init(struct net *net);
int brcm_proc_rem_dev(struct net_device *brcmdev);
int brcm_proc_add_dev(struct net_device *brcmdev);
void brcm_proc_cleanup(struct net *net);

#endif

