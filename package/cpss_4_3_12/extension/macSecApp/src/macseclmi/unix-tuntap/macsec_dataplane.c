/**

   @copyright
   Copyright (c) 2020, Rambus Inc. All rights reserved.

   File: macsec_dataplane.c

   @description
   MACsec LMI and data plane based on UNIX TUN/TAP devices.
*/

#include "macseclmi.h"
#include "macseclmi_sysdep.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/uio.h>

#include "lmi_message.h"

#include "macsec_dataplane.h"

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

static void
send_fd(int s, int fd);

/*
 * Initialize/uninitialize a peer object. init_peer() returns 0 if
 * failed and leaves peer in a partially initialized state to be
 * cleared using uninit_peer().
 */
static int init_peer(
  MacsecPeer *peer, MacsecIntf *intf, unsigned portid,
  const MacsecAddress *peer_address, unsigned peer_portid);
static void uninit_peer(MacsecPeer *peer);

/*
 * Thread init/uninit. init_thread() returns 0 if failed and leaves
 * thread in a partially initialized state to be cleared using
 * uninit_thread().
 */
static int init_thread(MlThread *t, void *(*func)(void *arg), void *arg);
static void uninit_thread(MlThread *t);

/*
 * Mutex init/uninit/lock/unlock.
 */
static int init_mutex(MlMutex *m);
static void uninit_mutex(MlMutex *m);
static void lock_mutex(MlMutex *m);
static void unlock_mutex(MlMutex *m);

/*
 * Write/read a signalling pipe.
 */
static void write_pipe(int (*fildes)[2]);
static void read_pipe(int (*fildes)[2]);

/*
 * Pipe init/uninit. init_pipe() returns 0 if failed and leaves pipe
 * in a partially initialized state to be cleared using uninit_pipe().
 */
static int init_pipe(int (*fildes)[2]);
static void uninit_pipe(int (*fildes)[2]);

/*
 * Interface manager thread routine.
 */
static void *manage_intf(void *arg);

/*
 * Add the peer to the select() read fd set of the associated ethernet
 * interface object.
 */
static void add_peer_fd(MacsecPeer *peer);

/*
 * Handle select() results for interfaces and peers.
 */
static void handle_intf(MacsecIntf *intf);
static void handle_intf_uncontrolled(MacsecIntf *intf);
static void handle_intf_ethif(MacsecIntf *intf);
static void handle_intf_tuntap(MacsecIntf *intf);
static void handle_peer(MacsecPeer *peer);

/*
 * Update the TUN/TAP interface according to controlled port MAC state
 * variables, most notably MAC_Operational.
 */
static void update_tuntap(MacsecIntf *intf, unsigned portid);

/*
 * Get pointer to the controlled port object of the real port (portid
 * less than 2) or a virtual port (portid greater than one).
 */
static MlPort *get_port(const MacsecIntf *intf, unsigned portid);

/*
 * Update octet and packet counters for a successful frame in or out.
 */
static void count_in(
  MacsecPortStat *pstat, const unsigned char *buf, unsigned len);
static void count_out(
  MacsecPortStat *pstat, const unsigned char *buf, unsigned len);

/*
 * Frame ring operations.
 */
static MlFrameEntry *put_frame_begin(MlFrameRing *ring);
static void put_frame_end(MlFrameRing *ring, MlFrameEntry *e);
static MlFrameEntry *get_frame_begin(MlFrameRing *ring);
static void get_frame_end(MlFrameRing *ring, MlFrameEntry *e);

/*
 * Error output.
 */
static void error_intf(const MacsecIntf *intf, const char *fmt, ...)
     __attribute__ ((format (printf, 2, 3)));
static void error_begin(const char *fmt, ...)
     __attribute__ ((format (printf, 1, 2)));
static void error_end(const char *fmt, va_list ap);
static void error_end_arg(int error, const char *fmt, va_list ap);

/*
 * Default EAPOL multicast address.
 */
static const MacsecAddress
default_multicast_address = {.buf = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03}};

/*
 * Broadcast address.
 */
static const MacsecAddress
broadcast_address = {.buf = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

/*
 * Miscellaneous flags controlled by the application.
 */
static unsigned flags;

/*
 * MACsec data plane context.
 */
typedef struct MacsecContextS
{
  /* the list for interfaces*/
  MacsecListLink interfaces;

  int loop;
  pthread_t dp_thread;
  int fd;

  uint32_t peer_id_next;

} MacsecContext;


/*
 * MACsec data plane context.
*/
static MacsecContext *macsec_context;

static MacsecIntf *
macsec_get_intf(
        MacsecIntfId intf_id)
{
  MacsecIntf *intf;

  intf =
      macsec_list_next(
              &macsec_context->interfaces,
              offsetof(MacsecIntf, link),
              NULL);

  while (intf != NULL && intf->ifindex != intf_id)
    {
      intf =
          macsec_list_next(
              &macsec_context->interfaces,
              offsetof(MacsecIntf, link),
              intf);
    }

  return intf;
}

static MacsecIntf *
macsec_get_intf_by_name(
        const char *ifname)
{
    MacsecIntf *intf;

    intf =
        macsec_list_next(
                &macsec_context->interfaces,
                offsetof(MacsecIntf, link),
                NULL);

    while (intf != NULL && strcmp(intf->ifname, ifname) != 0)
    {
        intf =
            macsec_list_next(
                    &macsec_context->interfaces,
                    offsetof(MacsecIntf, link),
                    intf);
    }

    return intf;
}

static MacsecPeer *
macsec_get_peer(
        MacsecPeerId peer_id)
{
  MacsecIntf *intf = NULL;

  intf =
    macsec_list_next(
            &macsec_context->interfaces,
            offsetof(MacsecIntf, link),
            NULL);

  while (intf != NULL)
    {
      MacsecPeer *peer = NULL;

      while ((peer = ml_peers_next(&intf->peers, peer)))
      {
        if (peer->peer_id == peer_id)
          return peer;
      }

      intf =
        macsec_list_next(
                &macsec_context->interfaces,
                offsetof(MacsecIntf, link),
                intf);
    }

  return NULL;
}

/*
 * Data plane api implementation functions.
 */

static void
dp_get_capabilities(
        MacsecCapabilities *capabilities)
{
  softsec_get_capabilities(capabilities);
}

static void
dp_get_secy_ctrl(
        MacsecIntfId intf_id,
        unsigned portid,
        MacsecSecyCtrl *ctrl)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));
  softsec_get_secy_ctrl(&intf->soft, portid, ctrl);
}

static void
dp_set_secy_ctrl(
        MacsecIntfId intf_id,
        unsigned portid,
        const MacsecSecyCtrl *ctrl)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));
  softsec_set_secy_ctrl(&intf->soft, portid, ctrl);
}

static void
dp_set_authorization(
  MacsecIntfId intf_id,
  unsigned portid,
  const MacsecAuthData *auth_data)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  MlPort *port = get_port(intf, portid);
  port->auth_data = *auth_data;
}

static void
dp_get_multicast_address(
  MacsecIntfId intf_id,
  MacsecAddress *address)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  *address = intf->multicast_address;
}


static void
dp_set_multicast_address(
  MacsecIntfId intf_id,
  const MacsecAddress *address)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  intf->multicast_address = *address;
}

static void
dp_get_latest_an(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned *latest_an)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  if (intf != NULL)
    {
      softsec_get_latest_an(&intf->soft, portid, latest_an);
    }
}

static void
dp_get_transmit_sa_next_pn(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an,
  uint64_t *next_pn)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));
  lock_mutex(&intf->mutex);
  softsec_get_next_pn(&intf->soft, portid, an, next_pn);
  unlock_mutex(&intf->mutex);
}

static void
dp_destroy_peer(
  MacsecPeerId peer_id)
{
  MacsecPeer *peer = macsec_get_peer(peer_id);
  MacsecIntf *intf = peer->intf;

  lock_mutex(&intf->mutex);
  uninit_peer(peer);
  ml_peers_remove(&intf->peers, peer);
  unlock_mutex(&intf->mutex);
}


static MacsecPeerId
dp_create_peer(
  MacsecIntfId intf_id,
  unsigned portid,
  const MacsecAddress *peer_address,
  unsigned peer_portid)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  MacsecPeer *peer = NULL;

  assert(portid >= 1);
  assert(portid < sizeof intf->vport_peer / sizeof intf->vport_peer[0]);
  assert(portid <= 1 || intf->vport_peer[portid] == NULL);

  lock_mutex(&intf->mutex);

  if (!(peer = ml_peers_insert(&intf->peers)))
    {
      error_intf(intf, "too many MACsec peers");
      goto fail;
    }

  if (!init_peer(peer, intf, portid, peer_address, peer_portid))
    goto fail;

  unlock_mutex(&intf->mutex);
  return peer->peer_id;

 fail:
  if (peer)
    {
      uninit_peer(peer);
      ml_peers_remove(&intf->peers, peer);
    }
  unlock_mutex(&intf->mutex);

  return 0;
}

static void
dp_get_secy_stat(
        MacsecIntfId intf_id,
        unsigned portid,
        MacsecSecyStat *stat)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));
  softsec_get_secy_stat(&intf->soft, portid, stat);
}

static void
dp_get_intf_attributes(
  MacsecIntfId intf_id,
  unsigned *ifindex,
  MacsecAddress *address)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  if (intf != NULL)
  {
      *ifindex = intf->ifindex;
      *address = intf->address;
  }
}

MacsecPeerId
dp_next_peer(
  MacsecIntfId intf_id,
  MacsecPeerId prev)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  MacsecPeer *peer = macsec_get_peer(prev);

  if (!(peer = ml_peers_next(&intf->peers, peer)))
    return 0;

  return peer->peer_id;
}

static void
dp_get_peer_attributes(
  MacsecPeerId peer_id,
  unsigned *portid,
  MacsecAddress *peer_address,
  unsigned *peer_portid)
{
  MacsecPeer *peer = macsec_get_peer(peer_id);

  *portid = peer->portid;
  *peer_address = peer->peer_address;
  *peer_portid = peer->peer_portid;
}

static void
dp_get_receive_sc_stat(
  MacsecPeerId peer_id,
  MacsecReceiveScStat *stat)
{
  MacsecPeer *peer = macsec_get_peer(peer_id);

  softsec_get_receive_sc_stat(peer->soft, stat);
}

static void
dp_set_controlled_port_enabled(
  MacsecIntfId intf_id,
  unsigned portid,
  int value)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  lock_mutex(&intf->mutex);
  softsec_set_controlled_port_enabled(&intf->soft, portid, value);
  unlock_mutex(&intf->mutex);
}

static void
dp_get_transmit_sc_stat(
  MacsecIntfId intf_id,
  unsigned portid,
  MacsecTransmitScStat *stat)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));
  softsec_get_transmit_sc_stat(&intf->soft, portid, stat);
}

static void
dp_install_key(
  MacsecIntfId intf_id,
  unsigned portid,
  const MacsecKey *sak,
  const MacsecKi *ki,
  const MacsecSalt *salt)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  MlPort *port = get_port(intf, portid);
  SoftsecDkey *soft;

  lock_mutex(&intf->mutex);
  soft = softsec_install_key(&intf->soft, portid, sak, ki, salt);
  unlock_mutex(&intf->mutex);

  port->last_dkey.soft = soft;
}

static void
dp_create_transmit_sa(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an,
  uint64_t next_pn,
  MacsecKi *ki,
  const MacsecSsci *ssci)
{
  SoftsecDkey *soft_dkey;
  MacsecIntf *intf = macsec_get_intf(intf_id);

  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));

  soft_dkey = softsec_get_dkey(&intf->soft, portid, ki);
  if (!soft_dkey)
      return;

  lock_mutex(&intf->mutex);
  softsec_create_transmit_sa(
    &intf->soft, portid, an, next_pn, soft_dkey, ssci);
  unlock_mutex(&intf->mutex);
}

static void
dp_enable_transmit(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));

  lock_mutex(&intf->mutex);
  softsec_enable_transmit(&intf->soft, portid, an);
  unlock_mutex(&intf->mutex);
}

static void
dp_get_transmit_sa_stat(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an,
  MacsecTransmitSaStat *stat)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));
  softsec_get_transmit_sa_stat(&intf->soft, portid, an, stat);
}

static void
dp_create_receive_sa(
  MacsecPeerId peer_id,
  unsigned an,
  uint64_t lowest_pn,
  MacsecKi *ki,
  const MacsecSsci *ssci)
{
  MacsecPeer *peer = macsec_get_peer(peer_id);
  SoftsecDkey *soft_dkey;

  soft_dkey = softsec_get_dkey(&peer->intf->soft, peer->portid, ki);
  if (!soft_dkey)
      return;

  lock_mutex(&peer->intf->mutex);
  softsec_create_receive_sa(peer->soft, an, lowest_pn, soft_dkey, ssci);
  unlock_mutex(&peer->intf->mutex);
}

static void
dp_enable_receive(
  MacsecPeerId peer_id,
  unsigned an)
{
  MacsecPeer *peer = macsec_get_peer(peer_id);

  lock_mutex(&peer->intf->mutex);
  softsec_enable_receive(peer->soft, an);
  unlock_mutex(&peer->intf->mutex);
}

static void
dp_set_receive_sa_lowest_pn(
  MacsecPeerId peer_id,
  unsigned an,
  uint64_t lowest_pn)
{
  MacsecPeer *peer = macsec_get_peer(peer_id);

  lock_mutex(&peer->intf->mutex);
  softsec_set_lowest_pn(peer->soft, an, lowest_pn);
  unlock_mutex(&peer->intf->mutex);
}

static void
dp_get_receive_sa_stat(
  MacsecPeerId peer_id,
  unsigned an,
  MacsecReceiveSaStat *stat)
{
  MacsecPeer *peer = macsec_get_peer(peer_id);

  softsec_get_receive_sa_stat(peer->soft, an, stat);
}

static void
dp_get_common_port_enabled(
  MacsecIntfId intf_id,
  int *value)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  /* clear signalling pipe before reading status */
  read_pipe(&intf->upipe);
  *value = intf->common_stat.mac_operational;
}

static int
dp_send_uncontrolled(
        MacsecIntfId intf_id,
        const MacsecAddress *dst,
        const MacsecAddress *src,
        const unsigned char *msdubuf,
        unsigned msdulen)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  MlFrameEntry *e;

  lock_mutex(&intf->mutex);
  e = put_frame_begin(&intf->utxring);
  unlock_mutex(&intf->mutex);

  if (!e)
    {
      error_intf(intf, "uncontrolled transmit ring full");
      return 0;
    }

  if (12 + msdulen > sizeof e->buf)
    {
      error_intf(intf, "frame too large for uncontrolled transmit");
      return 0;
    }

  memcpy(e->buf, dst, 6);
  memcpy(e->buf + 6, src, 6);
  memcpy(e->buf + 12, msdubuf, msdulen);
  e->len = 12 + msdulen;

  lock_mutex(&intf->mutex);
  put_frame_end(&intf->utxring, e);
  unlock_mutex(&intf->mutex);

  write_pipe(&intf->thread.pipe);
  return 1;
}

static unsigned
dp_receive_uncontrolled(
        MacsecIntfId intf_id,
        MacsecAddress *dst,
        MacsecAddress *src,
        unsigned char *msdubuf,
        unsigned msdulen)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  MlFrameEntry *e;
  unsigned l;

  /* clear signalling pipe before reading buffer */
  read_pipe(&intf->upipe);

  lock_mutex(&intf->mutex);
  e = get_frame_begin(&intf->urxring);
  unlock_mutex(&intf->mutex);

  if (!e)
    return 0;

  if (e->len < 12)
  {
    get_frame_end(&intf->urxring, e);
    error_intf(intf, "truncated uncontrolled frame");
    return 0;
  }

  if (e->len > msdulen)
  {
    get_frame_end(&intf->urxring, e);
    error_intf(intf, "uncontrolled frame too large (%u bytes)", e->len);
    return 0;
  }

  memcpy(dst, e->buf, 6);
  memcpy(src, e->buf + 6, 6);
  memcpy(msdubuf, e->buf + 12, e->len - 12);
  l = e->len - 12;

  lock_mutex(&intf->mutex);
  get_frame_end(&intf->urxring, e);
  unlock_mutex(&intf->mutex);

  return l;
}

static void
dp_get_common_stat(
  MacsecIntfId intf_id,
  MacsecPortStat *stat)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);
  *stat = intf->common_stat;
}

static void
dp_get_uncontrolled_stat(
  MacsecIntfId intf_id,
  MacsecPortStat *stat)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  *stat = intf->uncontrolled_stat;
}

static void
dp_get_controlled_stat(
  MacsecIntfId intf_id,
  unsigned portid,
  MacsecPortStat *stat)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  softsec_get_port_stat(&intf->soft, portid, stat);
}

static int
dp_test(void)
{
  return softsec_test();
}

static void
dp_uninit_intf(
  MacsecIntfId intf_id)
{
  MacsecIntf *intf = macsec_get_intf(intf_id);

  macsec_list_remove(&intf->link);
  uninit_thread(&intf->thread);
  uninit_mutex(&intf->mutex);
  uninit_pipe(&intf->upipe);
  ml_peers_foreach(&intf->peers, uninit_peer);
  softsec_uninit_intf(&intf->soft);
  ml_uninit_tuntap(&intf->port.tuntap);
  ml_uninit_ethif(&intf->ethif);
  memset(intf, 0, sizeof *intf);
}

static MacsecIntfId
dp_init_intf(
  const MacsecConfig *config)
{
  MacsecIntf *intf;
  MacsecContext *context = macsec_context;
  assert(context != NULL);

  intf = macsec_get_intf_by_name(config->ifname);
  if (intf)
  {
      *config->fdescp = intf->upipe[0];

      write_pipe(&intf->upipe);

      return intf->ifindex;
  }

  intf = malloc(sizeof *intf);
  if (!intf)
      return 0;

  memset(intf, 0, sizeof *intf);
  ml_peers_init(&intf->peers);
  intf->context = context;
  macsec_list_append(&context->interfaces, &intf->link);

  strncpy(intf->ifname, config->ifname, sizeof intf->ifname);
  intf->ifname[sizeof intf->ifname - 1] = '\0';

  if (!ml_init_ethif(&intf->ethif, intf->ifname))
    goto fail;
  ml_get_ethif_attributes(&intf->ethif, &intf->ifindex, &intf->address.buf);

  intf->multicast_address = default_multicast_address;

  if (!ml_init_tuntap(&intf->port.tuntap, &intf->ethif, 1))
    goto fail;

  softsec_init_intf(&intf->soft, &intf->address);
  intf->port.last_dkey.soft = NULL;
  memset(&intf->port.last_dkey.ki, 0, sizeof intf->port.last_dkey.ki);

  if (!init_pipe(&intf->upipe))
    goto fail;

  if (!init_mutex(&intf->mutex))
    goto fail;

  if (!init_thread(&intf->thread, manage_intf, intf))
    goto fail;

  *config->fdescp = intf->upipe[0];

  return intf->ifindex;

 fail:
  dp_uninit_intf(intf->ifindex);
  return 0;
}

static void
dp_set_intf_flags(MacsecIntfId intf_id, unsigned flags_value)
{
  flags = flags_value;
}

static MacsecIntfId
dp_next_intf(
        MacsecIntfId prev)
{
  MacsecIntf *intf = macsec_get_intf(prev);
  MacsecIntfId next = 0;

  intf =
      macsec_list_next(
              &macsec_context->interfaces,
              offsetof(MacsecIntf, link),
              intf);

  if (intf != NULL)
    {
      next = intf->ifindex;
    }

  return next;
}

static void
dp_handle_send_uncontrolled(
  MacsecMessage *message,
  MacsecMessage *response)
{
  const MacsecAddress *dst = macsec_message_get(message, sizeof *dst);
  const MacsecAddress *src = macsec_message_get(message, sizeof *src);
  const unsigned *msdulen = macsec_message_get(message, sizeof *msdulen);
  const unsigned char *msdubuf  = macsec_message_get(message, *msdulen);

  if (!macsec_message_error(message))
  {
    int result =
      dp_send_uncontrolled(message->intf_id, dst, src, msdubuf, *msdulen);

    MACSEC_MESSAGE_ADD_COPY(response, result);
  }
}

static void
dp_handle_receive_uncontrolled(
        MacsecMessage *message,
        MacsecMessage *response)
{
  unsigned msdulen;

  MACSEC_MESSAGE_GET_COPY(message, msdulen);

  if (!macsec_message_error(message))
  {
    MacsecAddress *dst = macsec_message_add_space(response, sizeof *dst);
    MacsecAddress *src = macsec_message_add_space(response, sizeof *src);
    unsigned *rlen = macsec_message_add_space(response, sizeof *rlen);
    unsigned char *msdubuf = macsec_message_add_space(response, msdulen);

    *rlen = dp_receive_uncontrolled(
              message->intf_id, dst, src, msdubuf, msdulen);

    macsec_message_set_last_len(response, *rlen);
  }
}

static void
dp_handle_create_peer(
        MacsecMessage *message,
        MacsecMessage *response)
{
  MacsecPeerId peer_id;
  unsigned *portid, *peer_portid;
  MacsecAddress *peer_address;

  portid = macsec_message_get(message, sizeof *portid);
  peer_address = macsec_message_get(message, sizeof *peer_address);
  peer_portid = macsec_message_get(message, sizeof *peer_portid);

  if (!macsec_message_error(message))
      peer_id = dp_create_peer(
              message->intf_id, *portid, peer_address, *peer_portid);

  MACSEC_MESSAGE_ADD_COPY(response, peer_id);
}

static void
dp_handle_get_peer_attributes(
        MacsecMessage *message,
        MacsecMessage *response)
{
  unsigned *portid = macsec_message_add_space(response, sizeof *portid);
  MacsecAddress *peer_address =
      macsec_message_add_space(response, sizeof *peer_address);
  unsigned *peer_portid =
      macsec_message_add_space(response, sizeof *peer_portid);

  if (!macsec_message_error(response))
    {
      dp_get_peer_attributes(
              message->peer_id, portid, peer_address, peer_portid);
    }
}

static void
macsec_dataplane_control(
        int fd)
{
  int loop = 1;

  while (loop)
    {
      int return_fd = -1;

      MacsecMessage message;
      MacsecMessage response;

      char message_buffer[10*1024];
      char response_buffer[10*1024];

      macsec_message_clear(&message);
      MACSEC_MESSAGE_ADD(&message, message_buffer);

      macsec_message_receive(fd, &message);
      if (macsec_message_error(&message) || message.size == 0)
        {
          break;
        }

      macsec_message_clear(&response);
      macsec_message_set_buffer(
              &response,
              response_buffer,
              sizeof response_buffer);

      switch (message.req)
        {
          case MACSEC_DP_GET_CAPABILITIES:
            {
              MacsecCapabilities mc;

              dp_get_capabilities(&mc);

              MACSEC_MESSAGE_ADD_COPY(&response, mc);
              break;
            }

          case MACSEC_DP_GET_SECYCTRL:
            {
              const unsigned *portid;
              MacsecSecyCtrl ctrl;

              portid = macsec_message_get(&message, sizeof *portid);

              if (!macsec_message_error(&message))
                dp_get_secy_ctrl(message.intf_id, *portid, &ctrl);

              MACSEC_MESSAGE_ADD_COPY(&response, ctrl);
              break;
            }

          case MACSEC_DP_SET_SECYCTRL:
            {
              const unsigned *portid;
              const MacsecSecyCtrl *ctrl;

              portid = macsec_message_get(&message, sizeof *portid);
              ctrl = macsec_message_get(&message, sizeof *ctrl);

              if (!macsec_message_error(&message))
                dp_set_secy_ctrl(message.intf_id, *portid, ctrl);

              break;
            }

          case MACSEC_DP_GET_SECYSTAT:
            {
              const unsigned *portid;
              MacsecSecyStat stat;

              portid = macsec_message_get(&message, sizeof *portid);

              if (!macsec_message_error(&message))
                dp_get_secy_stat(message.intf_id, *portid, &stat);

              MACSEC_MESSAGE_ADD_COPY(&response, stat);
              break;
            }

          case MACSEC_DP_SET_AUTHORIZATION:
            {
              const unsigned *portid;
              const MacsecAuthData *auth_data;

              portid = macsec_message_get(&message, sizeof *portid);
              auth_data = macsec_message_get(&message, sizeof *auth_data);

              if (!macsec_message_error(&message))
                dp_set_authorization(message.intf_id, *portid, auth_data);

              break;
            }

          case MACSEC_DP_SEND_UNCONTROLLED:
            dp_handle_send_uncontrolled(&message, &response);
            break;

          case MACSEC_DP_RECEIVE_UNCONTROLLED:
            dp_handle_receive_uncontrolled(&message, &response);
            break;

          case MACSEC_DP_DESTROY_PEER:
            dp_destroy_peer(message.peer_id);
            break;

          case MACSEC_DP_CREATE_PEER:
            dp_handle_create_peer(&message, &response);
            break;

          case MACSEC_DP_NEXT_PEER:
            {
              MacsecPeerId next_peer_id =
                  dp_next_peer(message.intf_id, message.peer_id);

              MACSEC_MESSAGE_ADD_COPY(&response, next_peer_id);
              break;
            }

          case MACSEC_DP_GET_PEER_ATTRIBUTES:
            dp_handle_get_peer_attributes(&message, &response);
            break;

          case MACSEC_DP_GET_COMMON_STAT:
            {
              MacsecPortStat stat;

              dp_get_common_stat(message.intf_id, &stat);
              MACSEC_MESSAGE_ADD_COPY(&response, stat);
              break;
            }
          case MACSEC_DP_GET_UNCONTROLLED_STAT:
            {
              MacsecPortStat stat;

              dp_get_uncontrolled_stat(message.intf_id, &stat);
              MACSEC_MESSAGE_ADD_COPY(&response, stat);
              break;
            }
          case MACSEC_DP_GET_CONTROLLED_STAT:
            {
              MacsecPortStat stat;
              unsigned *portid;

              portid = macsec_message_get(&message, sizeof *portid);

              if (!macsec_message_error(&message))
                dp_get_controlled_stat(message.intf_id, *portid, &stat);

              MACSEC_MESSAGE_ADD_COPY(&response, stat);
              break;
            }
          case MACSEC_DP_GET_RECEIVE_SC_STAT:
            {
              MacsecReceiveScStat stat;

              dp_get_receive_sc_stat(message.peer_id, &stat);
              MACSEC_MESSAGE_ADD_COPY(&response, stat);
              break;
            }
          case MACSEC_DP_GET_TRANSMIT_SC_STAT:
            {
              MacsecTransmitScStat stat;
              unsigned *portid;

              portid = macsec_message_get(&message, sizeof *portid);

              if (!macsec_message_error(&message))
                dp_get_transmit_sc_stat(message.intf_id, *portid, &stat);

              MACSEC_MESSAGE_ADD_COPY(&response, stat);
              break;
            }
          case MACSEC_DP_GET_RECEIVE_SA_STAT:
            {
              MacsecReceiveSaStat stat;
              unsigned *an;

              an = macsec_message_get(&message, sizeof *an);

              if (!macsec_message_error(&message))
                dp_get_receive_sa_stat(message.peer_id, *an, &stat);

              MACSEC_MESSAGE_ADD_COPY(&response, stat);
              break;
            }

          case MACSEC_DP_GET_TRANSMIT_SA_STAT:
            {
              MacsecTransmitSaStat stat;
              unsigned *portid, *an;

              portid = macsec_message_get(&message, sizeof *portid);
              an = macsec_message_get(&message, sizeof *an);

              if (!macsec_message_error(&message))
                dp_get_transmit_sa_stat(message.intf_id, *portid, *an, &stat);

              MACSEC_MESSAGE_ADD_COPY(&response, stat);
              break;
            }
          case MACSEC_DP_SET_CONTROLLED_PORT_ENABLED:
            {
              unsigned *portid, *value;

              portid = macsec_message_get(&message, sizeof *portid);
              value = macsec_message_get(&message, sizeof *value);

              if (!macsec_message_error(&message))
                dp_set_controlled_port_enabled(
                        message.intf_id, *portid, *value);
              break;
            }
          case MACSEC_DP_ENABLE_TRANSMIT:
            {
              unsigned *portid, *an;

              portid = macsec_message_get(&message, sizeof *portid);
              an = macsec_message_get(&message, sizeof *an);

              if (!macsec_message_error(&message))
                dp_enable_transmit(message.intf_id, *portid, *an);

              break;
            }
          case MACSEC_DP_ENABLE_RECEIVE:
            {
              unsigned *an;
              an = macsec_message_get(&message, sizeof *an);

              if (!macsec_message_error(&message))
                dp_enable_receive(message.peer_id, *an);

              break;
            }
          case MACSEC_DP_CREATE_TRANSMIT_SA:
            {
              unsigned *portid, *an;
              uint64_t *next_pn;
              MacsecKi *ki;
              MacsecSsci *ssci;

              portid = macsec_message_get(&message, sizeof *portid);
              an = macsec_message_get(&message, sizeof *an);
              next_pn = macsec_message_get(&message, sizeof *next_pn);
              ki = macsec_message_get(&message, sizeof *ki);
              ssci = macsec_message_get(&message, sizeof *ssci);

              if (!macsec_message_error(&message))
                dp_create_transmit_sa(message.intf_id, *portid, *an, *next_pn, ki, ssci);

              break;
            }
          case MACSEC_DP_INSTALL_KEY:
            {
              unsigned *portid;
              MacsecKey *sak;
              MacsecKi *ki;
              MacsecSalt *salt;
              unsigned *has_key;

              portid = macsec_message_get(&message, sizeof *portid);
              sak = macsec_message_get(&message, sizeof *sak);
              ki = macsec_message_get(&message, sizeof *ki);
              has_key = macsec_message_get(&message, sizeof *has_key);
              salt = macsec_message_get(&message, sizeof *salt);

              if (!macsec_message_error(&message))
                {
                  if (!*has_key)
                    salt = NULL;

                  dp_install_key(message.intf_id, *portid, sak, ki, salt);
                }
              break;
            }
          case MACSEC_DP_CREATE_RECEIVE_SA:
            {
              unsigned *an;
              uint64_t *lowest_pn;
              MacsecKi *ki;
              MacsecSsci *ssci;

              an = macsec_message_get(&message, sizeof *an);
              lowest_pn = macsec_message_get(&message, sizeof *lowest_pn);
              ki = macsec_message_get(&message, sizeof *ki);
              ssci = macsec_message_get(&message, sizeof *ssci);

              if (!macsec_message_error(&message))
                dp_create_receive_sa(message.peer_id, *an, *lowest_pn, ki, ssci);

              break;
            }
          case MACSEC_DP_SET_RECEIVE_LOWEST_PN:
            {
              unsigned *an;
              uint64_t *lowest_pn;

              an = macsec_message_get(&message, sizeof *an);
              lowest_pn = macsec_message_get(&message, sizeof *lowest_pn);

              if (!macsec_message_error(&message))
                dp_set_receive_sa_lowest_pn(message.peer_id, *an, *lowest_pn);

              break;
            }
          case MACSEC_DP_GET_COMMON_PORT_ENABLED:
            {
              int value;

              dp_get_common_port_enabled(message.intf_id, &value);
              MACSEC_MESSAGE_ADD_COPY(&response, value);
              break;
            }
          case MACSEC_DP_GET_MULTICAST_ADDRESS:
            {
              MacsecAddress address;

              dp_get_multicast_address(message.intf_id, &address);
              MACSEC_MESSAGE_ADD_COPY(&response, address);

              break;
            }
          case MACSEC_DP_SET_MULTICAST_ADDRESS:
            {
              const MacsecAddress *address;
              address = macsec_message_get(&message, sizeof *address);

              if (!macsec_message_error(&message))
                dp_set_multicast_address(message.intf_id, address);

              break;
            }
          case MACSEC_DP_GET_LATEST_AN:
            {
              unsigned an, *portid;
              portid = macsec_message_get(&message, sizeof *portid);

              if (!macsec_message_error(&message))
                dp_get_latest_an(message.intf_id, *portid, &an);

              MACSEC_MESSAGE_ADD_COPY(&response, an);
              break;
            }
          case MACSEC_DP_GET_TRANSMIT_SA_NEXT_PN:
            {
              unsigned *an, *portid;
              uint64_t next_pn;
              portid = macsec_message_get(&message, sizeof *portid);
              an = macsec_message_get(&message, sizeof *an);

              if (!macsec_message_error(&message))
                dp_get_transmit_sa_next_pn(
                        message.intf_id, *portid, *an, &next_pn);

              MACSEC_MESSAGE_ADD_COPY(&response, next_pn);
              break;
            }
          case MACSEC_DP_GET_INTF_ATTRIBUTES:
            {
              unsigned ifindex;
              MacsecAddress address;

              dp_get_intf_attributes(message.intf_id, &ifindex, &address);

              MACSEC_MESSAGE_ADD_COPY(&response, ifindex);
              MACSEC_MESSAGE_ADD_COPY(&response, address);
              break;
            }
          case MACSEC_DP_INIT_INTF:
            {
              MacsecConfig config;
              const MacsecConfig *config_p;
              MacsecIntfId ID;

              config_p = macsec_message_get(&message, sizeof *config_p);
              if (!macsec_message_error(&message))
              {
                config = *config_p;
                config.fdescp = &return_fd;

                ID = dp_init_intf(&config);
              }
              MACSEC_MESSAGE_ADD_COPY(&response, ID);
              break;
            }
          case MACSEC_DP_UNINIT_INTF:
            {
              dp_uninit_intf(message.intf_id);
              break;
            }
          case MACSEC_DP_NEXT_INTF:
            {
              MacsecIntfId ID = dp_next_intf(message.intf_id);
              MACSEC_MESSAGE_ADD_COPY(&response, ID);
              break;
            }
          case MACSEC_DP_TEST:
            {
              int result = dp_test();
              MACSEC_MESSAGE_ADD_COPY(&response, result);
              break;
            }

        case MACSEC_DP_SET_INTF_FLAGS:
          {
            unsigned *flags = macsec_message_get(&message, sizeof *flags);

            if (!macsec_message_error(&message))
              dp_set_intf_flags(message.intf_id, *flags);
            break;
          }

        default:
            loop = 0;
            break;
        }

      macsec_message_send(fd, &response);
      if (return_fd >= 0)
        send_fd(fd, return_fd);

      if (macsec_message_error(&message))
        loop = 0;
    }
}

static void *
macsec_dataplane_thread(
        void *param)
{
  MacsecContext *context = param;

  while (context->loop)
    {
      int fd = accept(context->fd, NULL, NULL);

      if (fd < 0)
        {
          sleep(1);
          continue;
        }

      macsec_dataplane_control(fd);

      close(fd);
    }

  return context;
}

/*
 * Static functions.
 */

static int
init_peer(
  MacsecPeer *peer, MacsecIntf *intf, unsigned portid,
  const MacsecAddress *peer_address, unsigned peer_portid)
{
  MacsecContext *context = macsec_context;
  MacsecPeerId peer_id;
  peer->intf = intf;

  peer->soft =
    softsec_create_peer(&intf->soft, portid, peer_address, peer_portid);

  if (!peer->soft)
    {
      error_intf(intf, "cannot add MACsec peer");
      return 0;
    }

  peer->port.last_dkey.soft = NULL;
  memset(&peer->port.last_dkey.ki, 0, sizeof peer->port.last_dkey.ki);

  if (portid > 1)
    {
      if (!ml_init_tuntap(&peer->port.tuntap, &intf->ethif, portid))
        return 0;
      intf->vport_peer[portid] = peer;
    }

  peer->portid = portid;
  peer->peer_address = *peer_address;
  peer->peer_portid = peer_portid;

  do
  {
    peer_id = context->peer_id_next++;
  } while (peer_id == 0 || macsec_get_peer(peer_id) != NULL);

  peer->peer_id = peer_id;

  peer->valid = 1;
  return 1;
}

static void
uninit_peer(MacsecPeer *peer)
{
  MacsecIntf *intf = peer->intf;

  peer->valid = 0;

  if (peer->portid > 1)
    {
      intf->vport_peer[peer->portid] = NULL;
      ml_uninit_tuntap(&peer->port.tuntap);
    }

  peer->portid = 0;

  if (peer->soft)
    {
      softsec_destroy_peer(peer->soft);
      peer->soft = NULL;
    }

  peer->intf = NULL;
}

static int
init_mutex(MlMutex *m)
{
  int n;

  if ((n = pthread_mutex_init(&m->ident, NULL)))
    {
      ml_error_arg(n, "pthread_mutex_init");
      return 0;
    }
  m->created = 1;
  return 1;
}

static void
uninit_mutex(MlMutex *m)
{
  int n;

  if (m->created)
    {
      m->created = 0;
      if ((n = pthread_mutex_destroy(&m->ident)))
        ml_error_arg(n, "pthread_mutex_destroy");
    }
}

static void
lock_mutex(MlMutex *m)
{
  int n;

  if ((n = pthread_mutex_lock(&m->ident)))
    ml_error_arg(n, "pthread_mutex_lock");
}

static void
unlock_mutex(MlMutex *m)
{
  int n;

  if ((n = pthread_mutex_unlock(&m->ident)))
    ml_error_arg(n, "pthread_mutex_unlock");
}

static int
init_thread(MlThread *t, void *(*routine)(void *arg), void *arg)
{
  sigset_t oset, set;
  int n, blocked = 0;

  if (!init_pipe(&t->pipe))
    goto fail;

  /* set a temporary sigmask to be inherited by the new thread */
  if ((n = pthread_sigmask(SIG_BLOCK, NULL, &oset)))
    {
      ml_error_arg(n, "pthread_sigmask");
      goto fail;
    }
  sigemptyset(&set);
  sigaddset(&set, SIGHUP);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGQUIT);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  if ((n = pthread_sigmask(SIG_BLOCK, &set, NULL)))
    {
      ml_error_arg(n, "pthread_sigmask");
      goto fail;
    }
  blocked = 1;

  /* create the thread */
  if ((n = pthread_create(&t->ident, NULL, routine, arg)))
    {
      ml_error_arg(n, "pthread_create");
      goto fail;
    }

  /* restore sigmask of this thread */
  if ((n = pthread_sigmask(SIG_SETMASK, &oset, NULL)))
    {
      ml_error_arg(n, "pthread_sigmask");
      goto fail;
    }

  t->created = 1;
  return 1;

 fail:
  if (blocked && (n = pthread_sigmask(SIG_SETMASK, &oset, NULL)))
    ml_error_arg(n, "pthread_sigmask");
  return 0;
}

static void
uninit_thread(MlThread *t)
{
  int n;

  if (t->created)
    {
      t->created = 0;

      t->terminate = 1;
      write_pipe(&t->pipe);

      if ((n = pthread_join(t->ident, NULL)))
        ml_error("pthread_join");

      t->terminate = 0;
    }

  uninit_pipe(&t->pipe);
}

static int
init_pipe(int (*fildes)[2])
{
  int flags, i;

  if (pipe(*fildes) < 0)
    {
      ml_error("pipe");
      return 0;
    }
  /* assume descriptor 0 is not returned, for zero-initialization to work */
  assert((*fildes)[0] > 0 && (*fildes)[1] > 0);

  for (i = 0; i < 2; i++)
    {
      if ((flags = fcntl((*fildes)[i], F_GETFL, 0)) < 0 ||
          fcntl((*fildes)[i], F_SETFL, flags | O_NONBLOCK) < 0)
        {
          ml_error("fcntl");
          return 0;
        }
    }

  return 1;
}

static void
write_pipe(int (*fildes)[2])
{
  unsigned char b = 0;
  int n;

  if ((n = write((*fildes)[1], &b, 1)) < 0)
    ml_error("write");
  else if (n < 1)
    ml_error("short write");
}

static void
read_pipe(int (*fildes)[2])
{
  unsigned char b;
  int n;

  while (1) {
    if ((n = read((*fildes)[0], &b, 1)) < 0)
      {
        if (errno != EAGAIN)
          ml_error("read");
        else
          errno = 0;
        break;
      }
    if (n == 0)
      break;
  }
}

static void
uninit_pipe(int (*fildes)[2])
{
  int i;

  for (i = 1; i >= 0; i--)
    {
      if ((*fildes)[i] <= 0)
        continue;

      if (close((*fildes)[i]) < 0)
        ml_error("close");
      (*fildes)[i] = 0;
    }
}

static void *
manage_intf(void *arg)
{
  MacsecIntf *intf = arg;
  int pipefd, ethfd, tunfd;
  struct timeval tv;
  unsigned char b;
  int n;

  error_intf(intf, "interface thread starting");

  pipefd = intf->thread.pipe[0];
  ethfd = ml_ethif_fd(&intf->ethif);
  tunfd = ml_tuntap_fd(&intf->port.tuntap);

  while (!intf->thread.terminate)
    {
      lock_mutex(&intf->mutex);

      FD_ZERO(&intf->rfds);
      intf->nfds = 0;

      FD_SET(pipefd, &intf->rfds);
      if (pipefd >= intf->nfds)
        intf->nfds = pipefd + 1;
      FD_SET(ethfd, &intf->rfds);
      if (ethfd >= intf->nfds)
        intf->nfds = ethfd + 1;
      FD_SET(tunfd, &intf->rfds);
      if (tunfd >= intf->nfds)
        intf->nfds = tunfd + 1;

      ml_peers_foreach(&intf->peers, add_peer_fd);

      unlock_mutex(&intf->mutex);

      tv.tv_sec = 0;
      tv.tv_usec = 250000;
      if ((n = select(intf->nfds, &intf->rfds, NULL, NULL, &tv)) < 0)
        {
          ml_error("select");
          break;
        }

      if (FD_ISSET(pipefd, &intf->rfds))
        {
          while ((n = read(pipefd, &b, 1)) > 0)
            ;
          if (n < 0)
            {
              if (errno != EAGAIN)
                ml_error("read");
              errno = 0;
            }
        }

      lock_mutex(&intf->mutex);

      handle_intf(intf);
      ml_peers_foreach(&intf->peers, handle_peer);

      unlock_mutex(&intf->mutex);
    }

  error_intf(intf, "interface thread stopping");
  return NULL;
}

static void
add_peer_fd(MacsecPeer *peer)
{
  MacsecIntf *intf;
  int fd;

  if (!peer->valid)
    return;

  if (peer->portid <= 1)
    return;

  intf = peer->intf;

  fd = ml_tuntap_fd(&peer->port.tuntap);

  FD_SET(fd, &intf->rfds);

  if (fd >= intf->nfds)
    intf->nfds = fd + 1;
}

static void
handle_intf(MacsecIntf *intf)
{
  MacsecPortStat *cstat = &intf->common_stat;
  MacsecPortStat *ucstat = &intf->uncontrolled_stat;
  int up, fdx;

  softsec_set_sysuptime(&intf->soft, ml_uptime());

  if (!ml_get_ethif_status(&intf->ethif, &up, &fdx))
    {
      up = 0;
      fdx = 0;
    }

  if (up != cstat->mac_operational || fdx != cstat->oper_point_to_point_mac)
    {
      if (up && !cstat->mac_operational)
        error_intf(intf, "common port up");
      else if (!up && cstat->mac_operational)
        error_intf(intf, "common port down");

      cstat->mac_enabled = up;
      cstat->mac_operational = up;
      cstat->oper_point_to_point_mac = fdx;

      ucstat->mac_enabled = cstat->mac_enabled;
      ucstat->mac_operational = cstat->mac_operational;
      ucstat->oper_point_to_point_mac = cstat->oper_point_to_point_mac;

      softsec_set_common_mac_state(
        &intf->soft,
        cstat->mac_enabled,
        cstat->mac_operational,
        cstat->oper_point_to_point_mac);

      write_pipe(&intf->upipe);
    }

  handle_intf_uncontrolled(intf);
  handle_intf_ethif(intf);
  handle_intf_tuntap(intf);
}

static void
handle_intf_uncontrolled(MacsecIntf *intf)
{
  MlFrameEntry *e;

  while ((e = get_frame_begin(&intf->utxring)))
    {
      if (ml_write_ethif(&intf->ethif, e->buf, e->len) < 0)
        intf->uncontrolled_stat.if_out_errors++;
      else
        count_out(&intf->uncontrolled_stat, e->buf, e->len);
      get_frame_end(&intf->utxring, e);
    }
}

static void
handle_intf_ethif(MacsecIntf *intf)
{
  unsigned char *srcbuf = intf->srcbuf, *dstbuf = intf->dstbuf;
  unsigned srclen = sizeof intf->srcbuf, dstlen = sizeof intf->dstbuf;
  unsigned l, type, portid;
  MlPort *port;
  MlFrameEntry *e;
  int fd, n;

  fd = ml_ethif_fd(&intf->ethif);
  if (!FD_ISSET(fd, &intf->rfds))
    return;

  if ((n = ml_read_ethif(&intf->ethif, srcbuf, srclen)) < 0)
    return;

  if (n == 0)
    return;

  srclen = n;

  /* get ethertype, discard frame if there is no valid ethernet header */
  if (srclen < 14)
    {
      error_intf(intf, "received short frame");
      return;
    }
  type = (srcbuf[12] << 8) | srcbuf[13];

  count_in(&intf->common_stat, srcbuf, srclen);

  /* divert unicast or multicast EAPOL to uncontrolled port */
  if (type == 0x888e &&
      (!(srcbuf[0] & 1) ||
       !memcmp(&srcbuf[0], intf->multicast_address.buf, 6)))
    {
      if (intf->context && intf->context->loop == 0)
        {
          error_intf(intf, "discard frame when connection to app closed");
          intf->uncontrolled_stat.if_in_discards++;
        }
      else
      if (!(e = put_frame_begin(&intf->urxring)))
        {
          intf->uncontrolled_stat.if_in_discards++;
        }
      else
        {
          assert(sizeof srcbuf <= sizeof e->buf);
          memcpy(e->buf, srcbuf, srclen);
          e->len = srclen;
          put_frame_end(&intf->urxring, e);
          count_in(&intf->uncontrolled_stat, srcbuf, srclen);
          write_pipe(&intf->upipe);
        }
      return;
    }

  l = softsec_input(&intf->soft, &portid, dstbuf, dstlen, srcbuf, srclen);

  if (l <= 0)
    return;

  port = get_port(intf, portid);

  if (ml_write_tuntap(&port->tuntap, dstbuf, l) < 0)
    return;
}

static void
handle_intf_tuntap(MacsecIntf *intf)
{
  unsigned char *srcbuf = intf->srcbuf, *dstbuf = intf->dstbuf;
  unsigned srclen = sizeof intf->srcbuf, dstlen = sizeof intf->dstbuf;
  unsigned l;
  int fd, n;

  update_tuntap(intf, 1);

  fd = ml_tuntap_fd(&intf->port.tuntap);
  if (!FD_ISSET(fd, &intf->rfds))
    return;

  if ((n = ml_read_tuntap(&intf->port.tuntap, srcbuf, srclen)) < 0)
    return;

  if (n == 0)
    return;

  srclen = n;

  l = softsec_output(&intf->soft, 1, dstbuf, dstlen, srcbuf, srclen);

  if (l <= 0)
    return;

  if (ml_write_ethif(&intf->ethif, dstbuf, l) < 0)
    return;

  count_out(&intf->common_stat, dstbuf, l);
}

static void
handle_peer(MacsecPeer *peer)
{
  MacsecIntf *intf;
  unsigned char *srcbuf, *dstbuf;
  unsigned srclen, dstlen, l;
  int fd, n;

  if (!peer->valid)
    return;

  if (peer->portid <= 1)
    return;

  intf = peer->intf;

  srcbuf = intf->srcbuf;
  dstbuf = intf->dstbuf;
  srclen = sizeof intf->srcbuf;
  dstlen = sizeof intf->dstbuf;

  update_tuntap(intf, peer->portid);

  fd = ml_tuntap_fd(&peer->port.tuntap);
  if (!FD_ISSET(fd, &intf->rfds))
    return;

  if ((n = ml_read_tuntap(&peer->port.tuntap, srcbuf, srclen)) < 0)
    return;

  if (n == 0)
    return;

  srclen = n;

  l = softsec_output(
    &intf->soft, peer->portid, dstbuf, dstlen, srcbuf, srclen);

  if (l <= 0)
    return;

  if (ml_write_ethif(&intf->ethif, dstbuf, l) < 0)
    return;

  return;
}

static void
update_tuntap(MacsecIntf *intf, unsigned portid)
{
  MlPort *port = get_port(intf, portid);
  MacsecPortStat pstat;

  softsec_get_port_stat(&intf->soft, portid, &pstat);

  if (pstat.mac_operational && !port->tuntap_enabled)
    {
      ml_enable_tuntap(&port->tuntap);
      port->tuntap_enabled = 1;
      error_intf(intf, "port %u up", portid);
    }
  else if (!pstat.mac_operational && port->tuntap_enabled)
    {
      ml_disable_tuntap(&port->tuntap);
      port->tuntap_enabled = 0;
      error_intf(intf, "port %u down", portid);
    }
}

static MlPort *
get_port(const MacsecIntf *intf, unsigned portid)
{
  assert(portid != 0 && (portid <= 1 || intf->vport_peer[portid] != NULL));

  if (portid <= 1)
    return (void *)&intf->port;
  else
    return &intf->vport_peer[portid]->port;
}

static void
count_in(MacsecPortStat *pstat, const unsigned char *buf, unsigned len)
{
  pstat->if_in_octets += len;

  if (!(buf[0] & 1))
    pstat->if_in_ucast_pkts++;
  else if (!memcmp(buf, &broadcast_address, 6))
    pstat->if_in_broadcast_pkts++;
  else
    pstat->if_in_multicast_pkts++;
}

static void
count_out(MacsecPortStat *pstat, const unsigned char *buf, unsigned len)
{
  pstat->if_out_octets += len;

  if (!(buf[0] & 1))
    pstat->if_out_ucast_pkts++;
  else if (!memcmp(buf, &broadcast_address, 6))
    pstat->if_out_broadcast_pkts++;
  else
    pstat->if_out_multicast_pkts++;
}

static MlFrameEntry *
put_frame_begin(MlFrameRing *ring)
{
  MlFrameEntry *e;

  e = &ring->entry[ring->nextput];

  if (e->ready)
    return NULL;

  return e;
}

static void
put_frame_end(MlFrameRing *ring, MlFrameEntry *e)
{
  e->ready = 1;

  ring->nextput++;
  ring->nextput &= (1 << ML_FRAME_RING_INDEX_BITS) - 1;
}

static MlFrameEntry *
get_frame_begin(MlFrameRing *ring)
{
  MlFrameEntry *e;

  e = &ring->entry[ring->nextget];

  if (!e->ready)
    return NULL;

  return e;
}

static void
get_frame_end(MlFrameRing *ring, MlFrameEntry *e)
{
  e->ready = 0;

  ring->nextget++;
  ring->nextget &= (1 << ML_FRAME_RING_INDEX_BITS) - 1;
}

static void
error_intf(const MacsecIntf *intf, const char *fmt, ...)
{
  va_list ap;

  flockfile(stderr);

  error_begin("%s: ", intf->ifname);
  va_start(ap, fmt);
  error_end(fmt, ap);
  va_end(ap);

  funlockfile(stderr);
}


static void
error_begin(const char *fmt, ...)
{
  struct timespec ts = {0, 0};
  unsigned long s, h, m, ms;
  int errno_save;
  va_list ap;

  if ((flags & MACSEC_CONFIG_FLAGS_SUPPRESS_MESSAGES))
    return;

  errno_save = errno;

  ml_realtime(&ts);
  s = ts.tv_sec % 86400;
  h = s / 3600;
  s -= h * 3600;
  m = s / 60;
  s -= m * 60;
  ms = ts.tv_nsec / 1000000;

  fprintf(stderr, "%02lu:%02lu:%02lu.%03lu ", h, m, s, ms);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  if (ferror(stderr))
    clearerr(stderr);

  errno = errno_save;
}

static void
error_end_arg(int error, const char *fmt, va_list ap)
{
  char error_str[64];

  if ((flags & MACSEC_CONFIG_FLAGS_SUPPRESS_MESSAGES))
    return;

  vfprintf(stderr, fmt, ap);
  if (error)
    {
      strerror_r(error, error_str, sizeof error_str);
      fprintf(stderr, ": %s", error_str);
    }
  fputc('\n', stderr);

  if (ferror(stderr))
    clearerr(stderr);

  errno = 0;
}

static void
error_end(const char *fmt, va_list ap)
{
  error_end_arg(errno, fmt, ap);

  errno = 0;
}

int
macsec_dataplane_init(
        void)
{
  MacsecContext *context = macsec_context;
  int fd = -1;
  struct sockaddr_un addr;
  socklen_t addr_len;
  int status;

  if (context == NULL)
    {
      context = calloc(1, sizeof(MacsecContext));
    }

  if (context == NULL)
    {
      ml_error("calloc failed");
      return 0;
    }

  macsec_list_init(&context->interfaces);

  memset(&addr, 0, sizeof addr);
  memcpy(addr.sun_path, LMI_SOCKET_PATH, strlen(LMI_SOCKET_PATH));
  addr.sun_family = AF_UNIX;
  addr_len = sizeof(addr.sun_family) + strlen(addr.sun_path) + 1;

  fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (fd < 0)
    {
      ml_error("Failed to open socket");
        goto error;
    }

  if (bind(fd, (struct sockaddr *) &addr, addr_len) < 0)
    {
      /* Unlink local end and attempt to re-bind */
      if (errno != EADDRINUSE ||
          unlink(addr.sun_path) < 0 ||
          bind(fd, (struct sockaddr *) &addr, addr_len) < 0)
        {
          ml_error("Failed to bind socket to");
          goto error;
        }
    }

  if (listen(fd, 5) == -1)
    {
      ml_error("Failed to listen socket");
      goto error;
    }

  macsec_context = context;
  macsec_context->fd = fd;
  macsec_context->loop = 1;

  status =
    pthread_create(
            &context->dp_thread,
            NULL,
            macsec_dataplane_thread,
            context);

  if (status != 0)
    {
      ml_error_arg(status, "pthread_create()");
      goto error;
    }

  return 1;

error:
    if (fd >= 0)
    {
        close(fd);
    }

    (void) unlink(addr.sun_path);

    return -1;
}

void
macsec_dataplane_uninit(
        void)
{
  if (macsec_context != NULL)
    {
      int status;
      void *ret;

      macsec_context->loop = 0;
      status = pthread_join(macsec_context->dp_thread, &ret);
      if (status != 0)
        {
          ml_error_arg(status, "pthread_join()");
        }

      close(macsec_context->fd);

      unlink(LMI_SOCKET_PATH);

      free(macsec_context);

      macsec_context = NULL;
    }
}

static void
send_fd(int s, int fd)
{
  struct msghdr msg = {0};
  struct cmsghdr *cmsg;
  char buf[CMSG_SPACE(sizeof fd)];
  int data = 0;
  struct iovec msg_iov;

  msg.msg_control = buf;
  msg.msg_controllen = CMSG_SPACE(sizeof fd);

  cmsg = CMSG_FIRSTHDR(&msg);

  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof fd);

  memcpy(CMSG_DATA(cmsg), &fd, sizeof fd);

  msg.msg_controllen = cmsg->cmsg_len;

  msg_iov.iov_base = &data;
  msg_iov.iov_len = sizeof data;

  msg.msg_iov = &msg_iov;
  msg.msg_iovlen = 1;

  if (sendmsg(s, &msg, 0) < 0)
    {
      ml_error("sendmsg()");
    }
}
