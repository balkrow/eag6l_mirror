/**

   Interface to services dependent on UNIX system type.

   File: macseclmi_sysdep_types.h

   @description
   Interface to services dependent on UNIX system type, needed by
   MACsec LMI and data plane based on UNIX TUN/TAP devices, Linux
   types.

   @copyright
   Copyright (c) 2012 - 2013, Rambus Inc. All rights reserved.

*/

#ifndef _MACSECLMI_SYSDEP_TYPES_H
#define _MACSECLMI_SYSDEP_TYPES_H

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>

/*
 * Ethernet interface object.
 */
typedef struct {
  /* a packet(7) socket bound to the ethernet */
  int sock;
  /* name of the ethernet interface */
  char name[IFNAMSIZ];
  /* interface index */
  int ifindex;
  /* MAC address */
  struct sockaddr address;
  /* MTU */
  int mtu;
} MlEthif;

/*
 * TUN/TAP device object.
 */
typedef struct {
  /* a raw socket */
  int sock;
  /* file descriptor to the tunnel interface */
  int fd;
  /* name of the tunnel interface */
  char name[IFNAMSIZ];
  /* associated ethernet interface */
  const MlEthif *ethif;
} MlTuntap;

#endif /* _MACSECLMI_SYSDEP_TYPES_H */
