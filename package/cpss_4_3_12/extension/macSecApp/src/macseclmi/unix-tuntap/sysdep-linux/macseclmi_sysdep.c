/**

   @copyright
   Copyright (c) 2012 - 2021, Rambus Inc. All rights reserved.

   File: macseclmi_sysdep.c

   @description
   Interface to services dependent on UNIX system type, needed by
   MACsec LMI and data plane based on UNIX TUN/TAP devices, Linux
   implementation.

*/

#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200112L

#include "macseclmi_sysdep.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <linux/if_tun.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#include <gtOs/gtOsInit.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedMemory.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*
 * Constants.
 */
static const char *tundev = "/dev/net/tun";

/*
 * Public functions.
 */

int
ml_init_ethif(MlEthif *e, const char *ifname)
{
  struct ifreq ifr;
  struct sockaddr_ll sll;
  struct packet_mreq mreq;
  int flags;

  /* init memory */
  memset(e, 0, sizeof *e);

  /* get a packet(7) socket */
  if ((e->sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
      ml_error("socket");
      goto fail;
    }
  /* assume descriptor 0 is not returned, for zero-initialization to work */
  assert(e->sock > 0);

  /* set interface name */
  cpssOsSnprintf(e->name, sizeof e->name, "%s", ifname);
  memset(&ifr, 0, sizeof ifr);
  cpssOsSnprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s", e->name);

  /* get interface index */
  if (ioctl(e->sock, SIOCGIFINDEX, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCGIFINDEX)", e->name);
      goto fail;
    }
  e->ifindex = ifr.ifr_ifindex;

  /* get the MAC address of the ethernet interface */
  if (ioctl(e->sock, SIOCGIFHWADDR, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCGIFHWADDR)", e->name);
      goto fail;
    }
  if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
    {
      ml_error("%s: not an ethernet interface", e->name);
      goto fail;
    }
  if (IFHWADDRLEN != 6)
    {
      ml_error("%s: bad hardware address length", e->name);
      goto fail;
    }
  e->address.sa_family = ifr.ifr_hwaddr.sa_family;
  memcpy(e->address.sa_data, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);

  /* bump third octet to get a different MAC address */
  e->address.sa_data[2]++;

  /* get the MTU of the ethernet interface */
  if (ioctl(e->sock, SIOCGIFMTU, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCGIFMTU)", e->name);
      goto fail;
    }
  e->mtu = ifr.ifr_mtu;

  /* bind the packet socket to the ethernet interface */
  memset(&sll, 0, sizeof sll);
  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = e->ifindex;
  if (bind(e->sock, (void *)&sll, sizeof sll) < 0)
    {
      ml_error("%s: bind", e->name);
      goto fail;
    }

  /* enable promiscuous reception */
  memset(&mreq, 0, sizeof mreq);
  mreq.mr_ifindex = e->ifindex;
  mreq.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(
        e->sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof mreq) < 0)
    {
      ml_error("%s: setsockopt", e->name);
      goto fail;
    }

  /* put the socket into nonblocking mode */
  if ((flags = fcntl(e->sock, F_GETFL, 0)) < 0 ||
      fcntl(e->sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
      ml_error("%s: fcntl", e->name);
      goto fail;
    }

  return 1;

 fail:
  ml_uninit_ethif(e);
  return 0;
}

void
ml_uninit_ethif(MlEthif *e)
{
  if (e->sock > 0)
    {
      if (close(e->sock))
        ml_error("close");
      e->sock = 0;
    }

  memset(e, 0, sizeof *e);
}

void
ml_get_ethif_attributes(
  MlEthif *e, int *ifindex, unsigned char (*address)[6])
{
  *ifindex = e->ifindex;
  memcpy(address, e->address.sa_data, IFHWADDRLEN);
}

int
ml_get_ethif_status(MlEthif *e, int *up, int *fdx)
{
  struct ifreq ifr;
  struct ethtool_cmd ec;
  struct ethtool_value ev;

  memset(&ifr, 0, sizeof ifr);
  cpssOsSnprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s", e->name);

  ifr.ifr_data = &ev;
  ev.cmd = ETHTOOL_GLINK;
  if (ioctl(e->sock, SIOCETHTOOL, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCETHTOOL, GLINK)", e->name);
      return 0;
    }

  if (ev.data)
    *up = 1;
  else
    *up = 0;

  ifr.ifr_data = &ec;
  ec.cmd = ETHTOOL_GSET;
  if (ioctl(e->sock, SIOCETHTOOL, &ifr) < 0)
    {
      if (errno != EOPNOTSUPP)
        {
          ml_error("%s: ioctl(SIOCETHTOOL, GSET)", e->name);
          return 0;
        }
      errno = 0;
      ec.duplex = DUPLEX_HALF;
    }

  if (ec.duplex == DUPLEX_FULL)
    *fdx = 1;
  else
    *fdx = 0;

  return 1;
}

int
ml_init_tuntap(MlTuntap *t, const MlEthif *e, unsigned portid)
{
  struct ifreq ifr;
  int flags;
  int format_ret;

  /* init memory */
  memset(t, 0, sizeof *t);

  /* get a socket */
  if ((t->sock = socket(AF_PACKET, SOCK_RAW, 0)) < 0)
    {
      ml_error("socket");
      goto fail;
    }

  /* make tunnel interface name and prepare ifreq */
  format_ret = cpssOsSnprintf(t->name, sizeof t->name, "%ssec%u", e->name, portid);
  if (format_ret >= sizeof t->name)
    {
      ml_error("tunnel interface name for %s is too long", e->name);
      goto fail;
    }
  memset(&ifr, 0, sizeof ifr);
  cpssOsSnprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s", t->name);

  /* get a tunnel interface */
  if ((t->fd = open(tundev, O_RDWR)) < 0)
    {
      ml_error("%s: open", tundev);
      goto fail;
    }
  /* assume descriptor 0 is not returned, for zero-initialization to work */
  assert(t->fd > 0);

  /* set name, type and packet format of the tunnel interface */
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(t->fd, TUNSETIFF, &ifr) < 0)
    {
      ml_error("%s: ioctl(TUNSETIFF,%s)", tundev, t->name);
      goto fail;
    }

  /* disable checksumming on the tunnel interface */
  if (ioctl(t->fd, TUNSETNOCSUM, 1) < 0)
    {
      ml_error("%s: ioctl(TUNSETNOCSUM)", t->name);
      goto fail;
    }

  /* ensure if down for MAC address change */
  if (ioctl(t->sock, SIOCGIFFLAGS, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCGIFFLAGS)", t->name);
      goto fail;
    }
  ifr.ifr_flags &= ~IFF_UP;
  if (ioctl(t->sock, SIOCSIFFLAGS, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCSIFFLAGS)", t->name);
      goto fail;
    }

  /* copy the ethernet's MAC address and MTU minus maximal MACsec
     SecTAG plus ICV length to the tunnel interface */
  ifr.ifr_hwaddr.sa_family = e->address.sa_family;
  memcpy(ifr.ifr_hwaddr.sa_data, e->address.sa_data, IFHWADDRLEN);
  if (ioctl(t->sock, SIOCSIFHWADDR, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCSIFHWADDR)", t->name);
      goto fail;
    }
  ifr.ifr_mtu = e->mtu;
  if (ifr.ifr_mtu >= 32)
    ifr.ifr_mtu -= 32;
  if (ioctl(t->sock, SIOCSIFMTU, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCSIFMTU)", t->name);
      goto fail;
    }

  /* put the file descriptor into nonblocking mode */
  if ((flags = fcntl(t->fd, F_GETFL, 0)) < 0 ||
      fcntl(t->fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
      ml_error("%s: fcntl", t->name);
      goto fail;
    }

  t->ethif = e;
  ml_error("%s: attached %s", t->ethif->name, t->name);
  return 1;

 fail:
  ml_uninit_tuntap(t);
  return 0;
}

void
ml_uninit_tuntap(MlTuntap *t)
{
  if (t->ethif)
    ml_error("%s: detaching %s", t->ethif->name, t->name);

  if (t->fd > 0)
    {
      if (close(t->fd))
        ml_error("%s: close", tundev);
      t->fd = 0;
    }

  if (t->sock > 0)
    {
      if (close(t->sock))
        ml_error("close");
      t->sock = 0;
    }

  memset(t, 0, sizeof *t);
}

void
ml_enable_tuntap(MlTuntap *t)
{
  struct ifreq ifr;

  memset(&ifr, 0, sizeof ifr);
  cpssOsSnprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s", t->name);
  if (ioctl(t->sock, SIOCGIFFLAGS, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCGIFFLAGS)", t->name);
      return;
    }
  ifr.ifr_flags |= IFF_UP;
  if (ioctl(t->sock, SIOCSIFFLAGS, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCSIFFLAGS)", t->name);
      return;
    }
}

void
ml_disable_tuntap(MlTuntap *t)
{
  struct ifreq ifr;

  memset(&ifr, 0, sizeof ifr);
  cpssOsSnprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s", t->name);
  if (ioctl(t->sock, SIOCGIFFLAGS, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCGIFFLAGS)", t->name);
      return;
    }
  ifr.ifr_flags &= ~IFF_UP;
  if (ioctl(t->sock, SIOCSIFFLAGS, &ifr) < 0)
    {
      ml_error("%s: ioctl(SIOCSIFFLAGS)", t->name);
      return;
    }
}

int
ml_ethif_fd(const MlEthif *e)
{
  return e->sock;
}

int
ml_tuntap_fd(const MlTuntap *t)
{
  return t->fd;
}

int
ml_read_ethif(MlEthif *e, unsigned char *buf, unsigned len)
{
  struct sockaddr_ll sll;
  socklen_t sl;
  int n;

  sl = sizeof sll;
  if ((n = recvfrom(e->sock, buf, len, MSG_TRUNC, (void *)&sll, &sl)) < 0)
    {
      if (errno != EAGAIN)
        {
          ml_error("%s: recvfrom", e->name);
          return -1;
        }
      errno = 0;
      return 0;
    }
  if (n > len)
    {
      ml_error("%s: oversize receive", e->name);
      return -1;
    }
  if (n == 0)
    return 0;

  if (sll.sll_pkttype == PACKET_OUTGOING)
    return 0;

  return n;
}

int
ml_write_ethif(MlEthif *e, const unsigned char *buf, unsigned len)
{
  int n;

  if ((n = send(e->sock, buf, len, 0)) < 0)
    {
      ml_error("%s: send", e->name);
      return -1;
    }
  if (n < len)
    {
      ml_error("%s: short send", e->name);
      return -1;
    }
  return n;
}

int
ml_read_tuntap(MlTuntap *t, unsigned char *buf, unsigned len)
{
  int n;

  if ((n = read(t->fd, buf, len)) < 0)
    {
      if (errno != EAGAIN)
        {
          ml_error("%s: read", t->name);
          return -1;
        }
      errno = 0;
      return 0;
    }

  return n;
}

int
ml_write_tuntap(MlTuntap *t, const unsigned char *buf, unsigned len)
{
  int n;

  if ((n = write(t->fd, buf, len)) < 0)
    {
      ml_error("%s: write", t->name);
      return -1;
    }
  if (n < len)
    {
      ml_error("%s: short write", t->name);
      return -1;
    }
  return n;
}

void
ml_realtime(struct timespec *tp)
{
  if (clock_gettime(CLOCK_REALTIME, tp) < 0)
    {
      ml_error("clock_gettime");
      memset(tp, 0, sizeof *tp);
    }
}

unsigned
ml_uptime(void)
{
  struct timespec ts;
  unsigned u;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
    {
      ml_error("clock_gettime");
      return 0;
    }

  u = ts.tv_sec * 10;
  u += ts.tv_nsec / 100000000;
  return u;
}
