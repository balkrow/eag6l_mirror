/**

   Data types for MACsec LMI and data plane based on UNIX TUN/TAP
   devices.

   File: macseclmi_types.h

   @description

   @copyright
   Copyright (c) 2012 - 2021, Rambus Inc. All rights reserved.

*/


#ifndef _MACSECLMI_TYPES_H
#define _MACSECLMI_TYPES_H

#include <sys/select.h>
#include "macseclmi_sysdep_types.h"
#if 0 /* TODO - MACSEC_APP_DRIVER_SOFTSEC */
#include "softsec.h"
#endif

#include "macsec_types.h"
#include "macsec_list.h"
#include "macsec_coll.h"
#include <pthread.h>
#include <sys/types.h>


/*
 * Define max MACsec peers per interface.
 */
#define MACSEC_MAX_PEERS 31 /* SOFTSEC_MAX_PEERS  -- TODO - MACSEC_APP_DRIVER_SOFTSEC */

/*
 * Define max MACsec interfaces.
 */
#define MACSEC_MAX_INTERFACES 128

/*
 * Maximum size of an ethernet frame. XXX
 */
#define ML_MAX_FRAME_SIZE 1514

/*
 * Identifier type for interface.
 */
typedef uint32_t MacsecIntfId;

/*
 * Identifier type for peer.
 */
typedef uint32_t MacsecPeerId;

/*
 * Identifier type for data keys.
 */
typedef uint32_t MacsecDkeyId;

#define MACSEC_CONFIG_FLAGS_SUPPRESS_MESSAGES 0x0001

/*
 * Interface configuration data.
 */
typedef struct {
  /* interface name */
  char ifname[IFNAMSIZ];
  /* pointer to a variable to store a waitable file handle into. */
  int *fdescp;
  /* pointer to a variable to store a pointer to a flags variable
     into. */
} MacsecConfig;

/*
 * Frame ring entry.
 */
typedef struct {
  int ready;
  unsigned char buf[ML_MAX_FRAME_SIZE];
  unsigned int len;
} MlFrameEntry;

/*
 * Frame ring.
 */
#define ML_FRAME_RING_INDEX_BITS 3
typedef struct {
  MlFrameEntry entry[1 << ML_FRAME_RING_INDEX_BITS];
  unsigned nextput;
  unsigned nextget;
} MlFrameRing;

/*
 * Mutex object.
 */
typedef struct {
  /* mutex identifier */
  pthread_mutex_t ident;
  /* mutex created flag */
  unsigned created : 1;
} MlMutex;

/*
 * Thread object.
 */
typedef struct {
  /* thread identifier */
  pthread_t ident;
  /* signalling pipe */
  int pipe[2];
  /* thread created flag */
  unsigned created : 1;
  /* thread termination flag */
  unsigned terminate : 1;
} MlThread;

/*
 * Data key object.
 */
typedef struct {
#if 0 /* TODO - MACSEC_APP_DRIVER_SOFTSEC */
  /* protocol key handle */
  SoftsecDkey *soft;
#endif
  /* key identifier */
  MacsecKi ki;
} MacsecDkey;

/*
 * MACsec (real or virtual) controlled port object.
 */
typedef struct {
  /* TUN/TAP device object */
  MlTuntap tuntap;
  /* TUN/TAP enabled flag */
  unsigned tuntap_enabled : 1;
  /* authorization data */
  MacsecAuthData auth_data;
  /* the last installed SAK */
  MacsecDkey last_dkey;
} MlPort;

/*
 * MACsec peer object.
 */
typedef struct MacsecPeerS {
  /* associated interface */
  struct MacsecIntfS *intf;
  /* port identifier; greater than 1 indicates a virtual port */
  unsigned portid;
  /* MAC address of peer */
  MacsecAddress peer_address;
  /* port identifier of peer */
  unsigned peer_portid;
  /* 1 if object completely initialized */
  int valid;
  /* controlled port port object of a virtual port; unused if portid <= 1 */
  MlPort port;
#if 0 /* TODO - MACSEC_APP_DRIVER_SOFTSEC */
  /* protocol peer handle */
  SoftsecPeer *soft;
#endif
  /* peer id */
  MacsecPeerId peer_id;
} MacsecPeer;

/*
 * Collection of peer objects.
 */
MACSEC_COLL(MlPeerColl, ml_peers, MACSEC_MAX_PEERS, MacsecPeer)

typedef struct MacsecContextS MacsecContext;

/*
 * Interface object.
 */
typedef struct MacsecIntfS {
  /* interface name of the underlying ethernet interface */
  char ifname[IFNAMSIZ];
  /* interface index */
  int ifindex;
  /* MAC address */
  MacsecAddress address;
  /* EAPOL multicast address */
  MacsecAddress multicast_address;
  /* ethernet interface object */
  MlEthif ethif;
  /* count of pointers to this object */
  unsigned refcount;
#if 0 /* TODO - MACSEC_APP_DRIVER_SOFTSEC */
  /* protocol state */
  SoftsecIntf soft;
#endif
  /* peer objects */
  MlPeerColl peers;
  /* port identifier to virtual port peer mapping (0 and 1 unused) */
  MacsecPeer *vport_peer[2 + MACSEC_MAX_PEERS];
  /* controlled port object of the real port */
  MlPort port;
  /* status variables of the common port */
  MacsecPortStat common_stat;
  /* status variables of the uncontrolled port */
  MacsecPortStat uncontrolled_stat;
  /* main thread vs. interface thread synchronization mutex */
  MlMutex mutex;
  /* thread object */
  MlThread thread;
  /* select() read fd set */
  fd_set rfds;
  /* select() fd count */
  int nfds;
  /* frame buffers */
  unsigned char srcbuf[ML_MAX_FRAME_SIZE];
  unsigned char dstbuf[ML_MAX_FRAME_SIZE];
  /* uncontrolled port signalling pipe */
  int upipe[2];
  /* ring of received frames for the uncontrolled port */
  MlFrameRing urxring;
  /* ring of frames to send from the uncontrolled port */
  MlFrameRing utxring;
  /* context that the interface is contained*/
  MacsecContext *context;
  MacsecListLink link;
} MacsecIntf;

#endif /* _MACSECLMI_TYPES_H */
