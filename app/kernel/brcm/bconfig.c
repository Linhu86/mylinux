/*
 * File: bconfig.c
 * Purpose: Tool to operate brcm devices in kernel. Support only add and remove
 *          operation.
 * Author: yoyo
 * Date: 2011-8-3
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <errno.h>
#include <string.h>
#include <linux/if_brcm.h>

static char *usage = 
  "Usage: add [interface-name] [brcm_port]\n"
  "rem [dev-name]";

int main(int argc, char *argv[])
{
  struct brcm_ioctl_args if_request;
  int port, fd;
  char *cmd, *ifname;

  if (argc != 3 && argc != 4) {
    fprintf(stderr, "%s\n", usage);
    return 0;
  }

  cmd = argv[1];
  memset(&if_request, 0, sizeof(struct brcm_ioctl_args));
  if (!strcasecmp(cmd, "add")) {
    ifname = argv[2];
    port = atoi(argv[3]);
    if_request.cmd = ADD_BRCM_CMD;
    strcpy(if_request.device1, ifname);
    if_request.u.port = port;
  } else if (!strcasecmp(cmd, "rem")) {
    ifname = argv[2];
    if_request.cmd = DEL_BRCM_CMD;
    strcpy(if_request.device1, ifname);
  } else {
    fprintf(stderr, "%s\n", usage);
    return 0;
  }
  
  // open fd
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "FATAL:  Couldn't open a socket..go figure!\n");
    exit(2);
  }   
  
  if (ioctl(fd, SIOCSIFBRCM, &if_request) < 0)
    fprintf(stderr,"ERROR: trying to add/rem brcm: %s\n", strerror(errno));
  return 0;
}

