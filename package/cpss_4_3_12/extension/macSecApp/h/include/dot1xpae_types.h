/**

   IEEE 802.1X-2010 types.

   File: dot1xpae_types.h

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_TYPES_H
#define _DOT1XPAE_TYPES_H

/*
 * Get platform-dependent definitions of maximum numbers of various
 * objects.
 */
#include "dot1xpae_platform.h"

#include "dot1xpae_crypto.h"
#include "eapauth.h"
#include "macseclmi.h"
#include "dot1x_types.h"
#include "macsec_types.h"
#include "macsec_dict.h"
#include "macsec_list.h"
#include <stdint.h>

/*
 * Clock ticks per one second.
 */
#define DOT1X_TICKS_PER_SECOND 2

typedef enum
{
  DOT1X_CAK_CACHE_NONE,
  DOT1X_CAK_CACHE_INSERT,
  DOT1X_CAK_CACHE_DELETE,
}
Dot1xCakCacheOp;

/** Event set for dot1x_handle_events(). */
typedef struct {
  /** 0.5 periodic timer. */
  unsigned time_tick : 1;
  /** Common port link state chage, to be read using
      macsec_get_common_port_enabled(). */
  unsigned common_port_state : 1;
  /** EAPOL frame ready to be received from the uncontrolled port
      using macsec_receive_uncontrolled(). */
  unsigned uncontrolled_port_receive : 1;
  /** EAP state change, to be read using eap_state(). */
  unsigned eap_state : 1;
  /** EAP packet ready to retrieved using eap_produce_packet() and
      sent. */
  unsigned eap_transmit : 1;
} Dot1xEvents;

/** CA key name (CKN). */
typedef struct {
  /** Buffer containing the data. */
  unsigned char buf[32];
  /** Length of data in bytes. */
  unsigned len;
} Dot1xCkn;

/*
 * Integrity check value (ICV).
 */
typedef struct {
  unsigned char buf[16];
} Dot1xIcv;

/*
 * Member identifier (MI).
 */
typedef struct {
  unsigned char buf[12];
} Dot1xMemberIdentifier;

/*
 * Key identifier (KI).
 */
typedef struct {
  Dot1xMemberIdentifier mi;
  uint32_t kn;
} Dot1xKeyIdentifier;

/*
 * MKA algorithm.
 */
typedef enum {
  DOT1X_MKA_ALGORITHM_8021X_2009
} Dot1xMkaAlgorithm;

/*
 * Peer list entry in an MKA PDU
 */
typedef struct {
  Dot1xMemberIdentifier mi;
  uint32_t mn;
} Dot1xMkpduPeer;

/*
 * Peer list in an MKA PDU
 */
typedef struct {
  Dot1xMkpduPeer tab[MACSEC_MAX_PEERS];
  unsigned num;
} Dot1xMkpduPeerList;

/*
 * MKA PDU structure. See IEEE 802.1X-2010 for the meanings of the
 * fields and abbreviations.
 */
typedef struct {
  /* PDU contains SAK use parameters */
  unsigned has_sak_use : 1;
  /* PDU contains distributed SAK parameters */
  unsigned has_distributed_sak : 1;
  /* PDU contains no key information in distributed SAK */
  unsigned has_distributed_plaintext : 1;
  /* PDU contains the optional cipher suite parameter in distributed SAK */
  unsigned has_distributed_cs : 1;
  /* PDU contains distributed CAK parameters */
  unsigned has_distributed_cak : 1;
  /* PDU contains KMD parameters */
  unsigned has_kmd : 1;
  /* PDU contains announcement parameters */
  unsigned has_announcement : 1;
  /* PDU contains XPN parameters */
  unsigned has_xpn : 1;

  /* basic parameter: MKA version */
  unsigned version;
  /* basic parameter: key server priority */
  uint32_t key_server_priority;
  /* basic parameter: key server */
  unsigned key_server : 1;
  /* basic parameter: MACsec desired */
  unsigned macsec_desired : 1;
  /* basic parameter: MACsec capability */
  MacsecCapability macsec_capability;
  /* basic parameter: secure channel identifier */
  MacsecSci sci;
  /* basic parameter: member identifier */
  Dot1xMemberIdentifier mi;
  /* basic parameter: message number */
  uint32_t mn;
  /* basic parameter: algorithm agility */
  Dot1xMkaAlgorithm mka_algorithm;
  /* basic parameter: CAK name */
  Dot1xCkn ckn;

  /* SAK use parameter: latest key association number */
  unsigned lan;
  /* SAK use parameter: latest key tx */
  unsigned ltx : 1;
  /* SAK use parameter: latest key rx */
  unsigned lrx : 1;
  /* SAK use parameter: old key association number */
  unsigned oan;
  /* SAK use parameter: old key tx */
  unsigned otx : 1;
  /* SAK use parameter: old key rx */
  unsigned orx : 1;
  /* SAK use parameter: plain tx */
  unsigned ptx : 1;
  /* SAK use parameter: plain rx */
  unsigned prx : 1;
  /* SAK use parameter: delay protect */
  unsigned dp : 1;
  /* SAK use parameter: latest key identifier */
  Dot1xKeyIdentifier lki;
  /* SAK use parameter: latest key lowest acceptable packet number */
  uint64_t llpn;
  /* SAK use parameter: old key identifier */
  Dot1xKeyIdentifier oki;
  /* SAK use parameter: old key lowest acceptable packet number */
  uint64_t olpn;

  /* distributed SAK parameter: distributed AN */
  unsigned distributed_an;
  /* distributed SAK parameter: confidentiality offset */
  MacsecConfidentialityOffset distributed_co;
  /* distributed SAK parameter: key number */
  uint32_t distributed_kn;
  /* distributed SAK parameter: MACsec cipher suite (if other than default) */
  MacsecCipherSuite distributed_cs;
  /* distributed SAK parameter: AES key wrap of SAK */
  Dot1xCryptoWrap distributed_sak;

  /* distributed CAK parameter: AES key wrap of CAK */
  Dot1xCryptoWrap distributed_cak;
  /* distributed CAK parameter: CAK name */
  Dot1xCkn distributed_ckn;

  /* KMD parameter: key managemenet domain */
  Dot1xKmd kmd;

  /* announcement parameters */
  struct Dot1xAnpduS *announcement;

  /* live peer list */
  Dot1xMkpduPeerList lpl;

  /* potential peer list */
  Dot1xMkpduPeerList ppl;

  /* integrity check value */
  Dot1xIcv icv;

  /* XPN parameter: MKA suspension time */
  unsigned xpn_mst;
  /* XPN parameter: latest key lowest acceptable packet number
     (most significant 32 bits) */
  uint32_t xpn_llpn;
  /* XPN parameter: old key lowest acceptable packet number
     (most significant 32 bits) */
  uint32_t xpn_olpn;

} Dot1xMkpdu;

/*
 * Per-network information in a network announcement.
 */
typedef struct {
  /* PDU contains access information */
  unsigned has_access_information : 1;
  /* PDU contains supported ciphersuites */
  unsigned has_cipher_suites : 1;
  /* PDU contains KMD */
  unsigned has_kmd : 1;
  /* network identity (zero for the default network) */
  Dot1xNid nid;
  /** access information */
  Dot1xAccessInformation access_information;
  /* supported ciphersuites */
  MacsecCipherCapability cipher_suites[MACSEC_CIPHER_SUITE_COUNT];
  /* key managemement domain */
  Dot1xKmd kmd;
} Dot1xAnpduNetwork;

/*
 * Network announcement PDU (ANPDU) structure.
 */
typedef struct Dot1xAnpduS {
  /* default network information (TLVs outside a NID TLV set */
  Dot1xAnpduNetwork default_network;
  /* network information */
  Dot1xAnpduNetwork network_tab[DOT1X_MAX_NETWORKS];
  unsigned network_num;
} Dot1xAnpdu;

/*
 * Per-network information in a network announcement request.
 */
typedef struct {
  /* PDU contains access information */
  unsigned has_access_information : 1;
  /* network identity (zero-length string for the default network) */
  Dot1xNid nid;
  /* access information */
  Dot1xAccessInformation access_information;
} Dot1xAnpduReqNetwork;

/*
 * Network announcement request structure.
 */
typedef struct Dot1xAnpduReqS {
  /* parent interface */
  struct Dot1xIntfS *intf;
  /* network information */
  Dot1xAnpduReqNetwork network_tab[DOT1X_MAX_NETWORKS];
  unsigned network_num;
} Dot1xAnpduReq;

/*
 * Supplicant/authenticator PACP instance in an EAP side object.
 */
typedef struct {
  /* parent EAP side */
  struct Dot1xPacpSideS *side;
  /* associated port object */
  struct Dot1xPortS *port;
  /* partner MAC address */
  MacsecAddress partner;
  /* associated network, if any, and list link */
  struct Dot1xNetworkS *network;
  MacsecListLink network_link;
  /* associated CAK, if any, and list link */
  struct Dot1xCakS *cak;
  MacsecListLink cak_link;
  /* EAP conversation handle, or NULL of no conversation in progress */
  EapConversation *eap_conversation;
  /* state machine state */
  enum {
    DOT1X_PACP_BEGIN,
    DOT1X_PACP_INITIALIZE,
    DOT1X_PACP_LOGOFF,
    DOT1X_PACP_HELD,
    DOT1X_PACP_UNAUTHENTICATED,
    DOT1X_PACP_AUTHENTICATING,
    DOT1X_PACP_AUTHENTICATED
  } state;
  /* PACP/Logon communication variables */
  unsigned enabled : 1;
  unsigned authenticated : 1;
  unsigned failed : 1;
  /* PACP/higher layer communication variables */
  unsigned eap_timeout : 1;
  unsigned eap_fail : 1;
  unsigned eap_success : 1;
  /* PACP/EAPOL communication variables */
  unsigned rx_eapol_start : 1;
  unsigned rx_eapol_logoff : 1;
  /* in addition, port_enabled is used by PACP */
  /* PACP internal variables */
  unsigned retry_count;
  unsigned held_while_ticks;
  unsigned quiet_while_ticks;
  unsigned reauth_when_ticks;
} Dot1xPacp;

/*
 * Dictionary of PACP instances with partner MAC address as the key.
 */
MACSEC_DICT(
  Dot1xPacps, dot1x_pacps, DOT1X_MAX_PACPS,
  Dot1xPacp, pacp, MacsecAddress, partner,
  pacp->partner = *partner,
  memcmp(&pacp->partner, partner, sizeof pacp->partner))

/*
 * PACP/Logon communication variables pertaining to all supplicant or
 * all authenticator PACPs as a group instead of individual PACPs.
 */
typedef struct Dot1xPacpSideS {
  /* parent interface */
  struct Dot1xIntfS *intf;
  /* PACP dictionary */
  Dot1xPacps pacps;
  /* pointer to last authenticated PACP, if any */
  Dot1xPacp *last_authenticated_pacp;
  /* PACP/Logon communication variables */
  unsigned authenticated_count;
  unsigned enabled_count;
  unsigned enabled : 1;
  unsigned authenticated : 1;
  unsigned failed : 1;
  unsigned failed_save : 1;
} Dot1xPacpSide;

/*
 * Announced network in an interface.
 */
typedef struct Dot1xNetworkS {
  /* parent interface */
  struct Dot1xIntfS *intf;
  /* network identifier */
  Dot1xNid nid;
  /* control variables */
  Dot1xNetworkCtrl ctrl;
  /* status variables */
  Dot1xNetworkStat stat;
  /* associated PACP instances, if any */
  MacsecListLink pacps;
  /* associated CAKs, if any */
  MacsecListLink caks;
  /* associated ports, if any */
  MacsecListLink ports;
  /* remaining lifetime ticks; 0 means infinite, 1 means expired */
  unsigned life_ticks;
} Dot1xNetwork;

/** CA key type. */
typedef enum {
  /** Pre-shared group CAK. */
  DOT1X_CAK_GROUP,
  /** Pre-shared pairwise CAK. */
  DOT1X_CAK_PAIRWISE,
  /** EAP-based pairwise CAK. */
  DOT1X_CAK_EAP,
  /** Distributed group CAK. */
  DOT1X_CAK_DISTRIBUTED
} Dot1xCakType;

/*
 * A CAK object in an interface.
 */
typedef struct Dot1xCakS {
  /* parent interface */
  struct Dot1xIntfS *intf;
  /* CAK name */
  Dot1xCkn ckn;
  /* key type describing the origin of the key */
  Dot1xCakType type;
  /* key value */
  MacsecKey key;
  /* remaining lifetime ticks; 0 means infinite, 1 means expired */
  unsigned life_ticks;
  /* the CAK is a (preshared or EAP-based) pairwise CAK */
  unsigned pairwise : 1;
  /* the CAK is a preshared (group or pairwise) CAK */
  unsigned preshared : 1;
  /* the CAK was derived/distributed/received in the EAP supplicant role */
  unsigned supplicant : 1;
  /* the CAK was derived/distributed/received in the EAP authenticator role */
  unsigned authenticator : 1;
  /* the CAK is a group CAK generated by the local station */
  unsigned generated : 1;
  /* the CAK is enabled */
  unsigned enable : 1;
  /* the EAP authentication associated with the CAK is no longer valid */
  unsigned eapfail : 1;
  /* peer MAC address valid */
  unsigned has_partner : 1;
  /* peer MAC address */
  MacsecAddress partner;
  /* key management domain */
  Dot1xKmd kmd;
  /* network identifier */
  Dot1xNid nid;
  /* associated network object, if any, and list link */
  struct Dot1xNetworkS *network;
  MacsecListLink network_link;
  /* associated PACP instances, if any */
  MacsecListLink pacps;
  /* authorization data (from EAP) */
  MacsecAuthData auth_data;
  /* associated real or virtual port, if any */
  struct Dot1xPortS *port;
  /* MKA instance, if any */
  struct Dot1xMkaS *mka;
  /* MKA instance being replaced, if any */
  struct Dot1xMkaS *replaced_mka;
  /* CKN for a possible parent pairwise CAK from where
   * this distributed CAK has been derived. Empty if no parent */ 
  Dot1xCkn parent_ckn;
  /* ticks to keep MKA active, 0 means infinite, 1 means expired */
  unsigned active_ticks;
  /* ticks to wait after MKA actor setup failure */
  unsigned held_while_ticks;
  /* ticks to keep a MKA instance being replaced */
  unsigned replace_ticks;
  /* Optional pre-shared key (CAK) activated callback */
  void (*activated_callback)(struct Dot1xCakS*);
  /* indicates whether a PSK CAK is actively in use
   * in a secure connection. Value is only valid for PSK CAKs. */
  unsigned sent_or_received_cak_or_sak;
  /* has the activated state already been reported */
  unsigned reported_activated;
  /* timer after last activity state change after which the state
   * is reported through callback */
  unsigned activated_callback_ticks;
} Dot1xCak;

/*
 * One second long packet number history.
 */
typedef struct {
  uint64_t ring[DOT1X_TICKS_PER_SECOND];
  unsigned last;
} Dot1xPnHistory;

/*
 * Real or virtual port data in an interface or in a peer object,
 * respectively.
 */
typedef struct Dot1xPortS {
  /* parent interface */
  struct Dot1xIntfS *intf;
  /* local port identifier */
  unsigned portid;

  /* connected network, if any */
  Dot1xNetwork *cnetwork;
  /* requested network, if any, and list link */
  Dot1xNetwork *rnetwork;
  MacsecListLink rnetwork_link;
  /* remaining ticks to keep requested network */
  unsigned rnetwork_ticks;

  /* announcement request with solicit received */
  unsigned announcement_solicited : 1;

  /* per-port PACP/Logon communication variables */
  unsigned supp_authenticate : 1;
  unsigned auth_authenticate : 1;

  /* Logon/CP communication variables */
  enum {
    DOT1X_CONNECT_PENDING,
    DOT1X_CONNECT_UNAUTHENTICATED,
    DOT1X_CONNECT_AUTHENTICATED,
    DOT1X_CONNECT_SECURE
  } connect;

  /* Logon/MKA communication variables */
  unsigned mka_failed : 1;
  unsigned mka_authenticated : 1;
  unsigned mka_secured : 1;

  /* MKA/CP communication variables */
  unsigned chgd_server : 1;
  unsigned elected_self : 1;
  MacsecAuthData *authorization_data;
  MacsecCipherSuite cipher_suite;
  MacsecConfidentialityOffset cipher_offset;
  unsigned new_sak : 1;
  Dot1xKeyIdentifier distributed_ki;
  unsigned distributed_an;
  Dot1xKeyIdentifier lki;
  unsigned lan;
  unsigned lrx : 1;
  unsigned ltx : 1;
  Dot1xKeyIdentifier oki;
  unsigned oan;
  unsigned orx : 1;
  unsigned otx : 1;
  unsigned using_receive_sas : 1;
  unsigned all_receiving : 1;
  unsigned server_transmitting : 1;
  unsigned using_transmit_sa : 1;
  unsigned new_info : 1;
  unsigned transmit_when_ticks;
  unsigned retire_when_ticks;

  /* CP/client communication variables */
  unsigned port_valid;

  /* CP/SecY communication variables */
  MacsecCipherSuite current_cipher_suite;
  MacsecConfidentialityOffset confidentiality_offset;
  MacsecValidateFrames validate_frames;
  unsigned controlled_port_enabled : 1;
  unsigned protect_frames : 1;
  unsigned replay_protect : 1;
  /* nonzero if the above are updated in the SecY */
  unsigned secy_updated : 1;

  /* CP machine state */
  enum {
    DOT1X_CP_BEGIN,
    DOT1X_CP_INIT,
    DOT1X_CP_CHANGE,
    DOT1X_CP_ALLOWED,
    DOT1X_CP_AUTHENTICATED,
    DOT1X_CP_SECURED,
    DOT1X_CP_RECEIVE,
    DOT1X_CP_RECEIVING,
    DOT1X_CP_READY,
    DOT1X_CP_TRANSMIT,
    DOT1X_CP_ABANDON,
    DOT1X_CP_TRANSMITTING,
    DOT1X_CP_RETIRE
  } cp_state;

  /* MKA/SecY communication variables */
  unsigned transmit_sa_in_use;
  /* per-tick history of transmit_sa_next_pn for one second for each AN */
  Dot1xPnHistory transmit_sa_next_pn_history[4];

  /* PN to use to create a new transmit SA after SAK change */
  uint64_t new_transmit_sa_next_pn;
  /* AN of created transmit SA */
  unsigned new_transmit_sa_an;
  /* AN of the transmit SA in use */
  unsigned transmit_sa_an;

  /* generated/received SAK */
  MacsecKey distributed_sak;
  /* MACsec installed key id */
  MacsecKi macsec_ki;
  /* nonzero if enableReceiveSAs() done */
  unsigned receive_sas_enabled : 1;

  /* CAK entry corresponding to distributed CAK */
  Dot1xCak *distributed_cak;
  /* the peer that distributed group CAK, if not locally generated */
  struct Dot1xPeerS *distributed_cak_source;

  /* lowest acceptable packet numbers of each AN */
  uint64_t lpn[4];

  /* next AN to use as local key server */
  unsigned next_an;

  /* MKA key server sources in priority order */
  MacsecListLink key_sources;
  /* pointer to principal MKA instance, if any */
  struct Dot1xMkaS *principal;

  /*  key_server */
  struct Dot1xMkaKeySourceS *key_server;

  /* MKA port level (as opposed to MKA instance level) peer station
     counters */
  unsigned peer_count;
  unsigned highest_peer_count;
  unsigned receiving_peer_count;
  unsigned using_peer_count;
  /* ticks to wait MKA authentication, 0 means infinite, 1 means expired */
  unsigned mka_fail_ticks;

  /* SAK regeneration throttle timer */
  unsigned sak_hold_ticks;

  /* Distribute SAK as keyserver only while set. */
  unsigned distribute_sak : 1;
  /* The suspendOnRequest control. */
  unsigned suspend_on_request : 1;

  /* The suspendedWhile value when suspension is active. */
  unsigned suspended_while;
  /* ticks for MKA suspension. */
  unsigned suspended_while_ticks;
  unsigned local_suspended_while_ticks;
  unsigned suspend_for;

  /* Port had peers on data plane and is resuming after suspension. */
  unsigned resuming_suspend : 1;

  unsigned can_suspend : 1;

  unsigned suspended : 1;

  unsigned has_sa : 1;

} Dot1xPort;

/*
 * Peer type according to whether the real port or a virtual port is
 * associated with the peer.
 */
typedef enum {
  DOT1X_PEER_REAL,
  DOT1X_PEER_VIRTUAL
} Dot1xPeerType;

/*
 * MACsec peer station record in an interface.
 */
typedef struct Dot1xPeerS {
  /* the port this peer is associated with; either the real port or
     the virtual port object in this peer */
  Dot1xPort *port;
  /* MAC address of peer station */
  MacsecAddress peer_address;
  /* port identifier of peer station */
  unsigned peer_portid;
  /* short SCI */
  MacsecSsci ssci;
  /* virtual port data, used iff virtual ports are enabled */
  Dot1xPort vport;
  /* number of all references (from virtual port CAKs or MKA peers) to
     this peer */
  unsigned reference_count;
  /* number of references from virtual port CAKs to this peer */
  unsigned logon_reference_count;
  /* number of references from live MKA peers to this peer */
  unsigned mka_reference_count;
  /* MACsec peer handle */
  MacsecPeerId macsec_peer;
  /* the peer is receiving using the current SAK */
  unsigned receiving : 1;
  /* inUse status of the latest receive SA */
  unsigned receive_sa_in_use : 1;
  /* has SA installed */
  unsigned has_sa : 1;
  /* AN of created transmit SA */
  unsigned new_receive_sa_an;
  /* latest LLPN received from the peer of the principal MKA instance */
  uint64_t llpn;
  /* LLPN changed and not yet acted upon */
  unsigned llpn_changed : 1;
} Dot1xPeer;

/*
 * Key server candidate (a local or remote MKA participant).
 */
typedef struct Dot1xMkaKeySourceS {
  MacsecListLink link;
  /* local MKA instance associated with this key source */
  struct Dot1xMkaS *mka;
  /* MKA peer (if remote key source) or NULL (if local key source) */
  struct Dot1xMkaPeerS *mka_peer;
  /* key server priority of the instance/peer */
  unsigned priority;
  /* SCI of the local/remote port in network representation */
  unsigned char sci[8];
  /* the MKA participant is based on a (preshared or EAP-based) pairwise CAK */
  unsigned pairwise : 1;
  /* the MKA participant is based on a CAK produced in the supplicant role */
  unsigned supplicant : 1;
  /* the live peer count of the key source's MKA instance */
  unsigned live_peer_count;
  /* willing to be key server */
  unsigned willing_key_server : 1;
} Dot1xMkaKeySource;

/*
 * MKA peer record in an MKA instance.
 */
typedef struct Dot1xMkaPeerS {
  /* parent MKA instance */
  struct Dot1xMkaS *mka;
  /* member identifier */
  Dot1xMemberIdentifier mi;
  /* message number */
  uint32_t mn;
  /* remaining time to expiry of potential peer status */
  unsigned potential_ticks;
  /* remaining time to expiry of live peer status */
  unsigned live_ticks;
  /* reference to a peer record */
  Dot1xPeer *peer;
  /* this peer as a key server candidate */
  Dot1xMkaKeySource key_source;
  /* state flags */
  unsigned live : 1;
  unsigned macsec : 1;
  /* suspendFor value in the last MKPDU */
  unsigned suspend_for;
} Dot1xMkaPeer;

/*
 * The next transmit packet number for a previously used SAK.
 */
typedef struct {
  /* member identifier */
  Dot1xMemberIdentifier mi;
  /* key number */
  uint32_t kn;
  /* next packet number to use for this SAK */
  uint64_t next_pn;
} Dot1xSakPn;

/*
 * Dictionary of MKA peers with member identifier as the key.
 */
MACSEC_DICT(
  Dot1xMkaPeers, dot1x_mka_peers, MACSEC_MAX_PEERS,
  Dot1xMkaPeer, mkap, Dot1xMemberIdentifier, mi,
  mkap->mi = *mi, memcmp(&mkap->mi, mi, sizeof mkap->mi))

/*
 * Dictionary of KI-PN pairs with member identifier as the key.
 */
MACSEC_DICT(
  Dot1xSakPns, dot1x_sak_pns, DOT1X_MAX_MKA_SAK_PNS,
  Dot1xSakPn, sak_pn, Dot1xMemberIdentifier, mi,
  sak_pn->mi = *mi, memcmp(&sak_pn->mi, mi, sizeof sak_pn->mi))

/*
 * MKA instance in an interface.
 */
typedef struct Dot1xMkaS {
  /* CAK information */
  Dot1xCak *cak;
  /* key exchange key */
  MacsecKey kek;
  /* integrity check key and MAC */
  MacsecKey ick;
  void *ick_mac;
  /* member identifier */
  Dot1xMemberIdentifier mi;
  /* member message number */
  uint32_t mn;
  /* next key number to use as key server */
  uint32_t kn;

  /* time to sending the next MKPDU */
  unsigned send_ticks;

  /* this instance as a key server candidate */
  Dot1xMkaKeySource key_source;

  /* this instances elected key_server */
  Dot1xMkaKeySource *key_server;

  /* MKA peers */
  Dot1xMkaPeers peers;
  /* MKA peer counters */
  unsigned peer_count;
  unsigned live_peer_count;
  unsigned macsec_peer_count;

  /* KI-nextPN records of past SAKs */
  Dot1xSakPns sak_pns;

  /* how many times left to send KMD plus one; 0 means do not send */
  unsigned kmd_count;

  /* condition flags */
  unsigned live_peers_changed : 1;
  unsigned pn_exhaust_pending : 1;
  unsigned duplicate : 1;
  unsigned replace : 1;
  unsigned send_mkpdu : 1;
  unsigned potential_principal : 1;

  /* A new peer without existing Dot1xPeer has become live. */
  unsigned suspension_unseen_peer : 1;
  unsigned keyserver_suspend_ticks;

} Dot1xMka;

/*
 * Dictionary of CAKs with CKN as the key.
 */
MACSEC_DICT(
  Dot1xCaks, dot1x_caks, DOT1X_MAX_MKA_INSTANCES,
  Dot1xCak, cak, Dot1xCkn, ckn,
  { memcpy(cak->ckn.buf, ckn->buf, ckn->len); cak->ckn.len = ckn->len; },
  cak->ckn.len != ckn->len ?
    (int)(cak->ckn.len - ckn->len) :
    memcmp(cak->ckn.buf, ckn->buf, cak->ckn.len))

/*
 * Collection of MKA instances.
 */
MACSEC_COLL(
  Dot1xMkas, dot1x_mkas, DOT1X_MAX_MKA_INSTANCES, Dot1xMka)

/*
 * Dictionary of peers with peer MAC address as the key.
 */
MACSEC_DICT(
  Dot1xPeers, dot1x_peers, MACSEC_MAX_PEERS,
  Dot1xPeer, peer, MacsecAddress, peer_address,
  peer->peer_address = *peer_address,
  memcmp(&peer->peer_address, peer_address, sizeof peer->peer_address))

/*
 * Dictionary of announced networks with NID as the key.
 */
MACSEC_DICT(
  Dot1xNetworks, dot1x_networks, DOT1X_MAX_NETWORKS,
  Dot1xNetwork, nw, Dot1xNid, nid,
  { memcpy(nw->nid.buf, nid->buf, nid->len); nw->nid.len = nid->len; },
  nw->nid.len != nid->len ?
    (int)(nw->nid.len - nid->len) :
    memcmp(nw->nid.buf, nid->buf, nw->nid.len))

/*
 * Interface data.
 */
typedef struct Dot1xIntfS {

  char ifname[IFNAMSIZ];

  /* EAP interface context */
  EapContext *eap_context;
  /* MACsec interface context */
  MacsecIntfId macsec_intf;
  /* random number generator and AES-CMAC data */
  Dot1xCryptoRandom crypto_random;
  Dot1xCryptoCmac crypto_cmac;
  /* local MAC address and interface index */
  MacsecAddress local_address;
  unsigned local_ifindex;
  /* short SCI */
  MacsecSsci ssci;
  /* MACsec cipher suite configuration */
  MacsecCipherSuite configured_cipher_suite;
  MacsecConfidentialityOffset configured_cipher_offset;

  /* real port data */
  Dot1xPort rport;

  /* default network */
  Dot1xNetwork dnetwork;
  /* announced network dictionary */
  Dot1xNetworks networks;

  /* selected network set by management */
  Dot1xNetwork *snetwork;

  /* CAK store */
  Dot1xCaks caks;

  /* PACP sides */
  Dot1xPacpSide supp;
  Dot1xPacpSide auth;
  /* PACP state change flag */
  unsigned pacp_changed : 1;

  /* interface state changed */
  unsigned changed : 1;

  /* CP state changed */
  unsigned cp_changed : 1;

  /* Logon/MKA communication variables */
  unsigned mka_enabled : 1;

  /* uncontrolled port status */
  unsigned port_enabled : 1;

  /* MSDU encode/decode buffer */
  unsigned char msdu[1514];
  /* MKA PDU buffer */
  Dot1xMkpdu mkpdu;
  /* announcement PDU buffer */
  Dot1xAnpdu anpdu;
  /* announcement request buffer */
  Dot1xAnpduReq anpdureq;

  /* MKA instances */
  Dot1xMkas mkas;
  /* MKA state changed */
  unsigned mka_changed : 1;

  /* Logon process state changed */
  unsigned logon_changed : 1;

  /* peer dictionary */
  Dot1xPeers peers;

  /* announcement request sent by listener */
  unsigned announcement_req_sent : 1;
  /* initial announcement after portEnable transition sent by announcer */
  unsigned initial_announcement_sent : 1;
  /* time to wait before sending next periodic announcement */
  unsigned announcement_ticks;
  /* max number of rate-limited sends left during current second */
  unsigned announcement_credit;

  /* diagnostic message enable bits */
  uint32_t messages;
  /* diagnostic message formatting buffer */
  char message_buf[256];
  /* message start and end positions in the buffer */
  int message_start;
  int message_end;

  /** Control variables. */
  Dot1xPaeCtrl ctrl;
  /** Status variables. */
  Dot1xPaeStat stat;

  int next_vportid;

  void (*cak_cache_callback)(Dot1xCakCacheOp op, const Dot1xCak *cak);

} Dot1xIntf;

/** Diagnostic message categories. */
typedef enum {
  /** Zero value with no effect. */
  DOT1X_MESSAGE_NONE,
  /** Software errors. */
  DOT1X_MESSAGE_ERROR,
  /** Packet discards. */
  DOT1X_MESSAGE_DISCARD,
  /** Uncontrolled and controlled port status events. */
  DOT1X_MESSAGE_PORT,
  /** Default or announced network create/destroy/status events. */
  DOT1X_MESSAGE_NETWORK,
  /** MACsec peer create/destroy. */
  DOT1X_MESSAGE_PEER,
  /** CA key create/destroy. */
  DOT1X_MESSAGE_CAK,
  /** SAK change. */
  DOT1X_MESSAGE_SAK,
  /** Key server change. */
  DOT1X_MESSAGE_KEY_SERVER,
  /** EAPOL PDUs. */
  DOT1X_MESSAGE_EAPOL,
  /** EAP packets. */
  DOT1X_MESSAGE_EAP,
  /** MKA PDUs. */
  DOT1X_MESSAGE_MKA,
  /** Announcement and announcement request PDUs. */
  DOT1X_MESSAGE_ANNOUNCEMENT,
  /** Enable full decodes of PDUs instead of compact decodes. */
  DOT1X_MESSAGE_PACKET_DETAIL,
  /** PACP state changes and other events. */
  DOT1X_MESSAGE_PACP,
  /** MKA actor create/destroy. */
  DOT1X_MESSAGE_MKA_ACTOR,
  /** MKA peer create/destroy. */
  DOT1X_MESSAGE_MKA_PEER,
  /** CP state changes. */
  DOT1X_MESSAGE_CP,
  /** Change of control variables. */
  DOT1X_MESSAGE_CONTROL,
  /** Changes of Logon/PACP communication variables. */
  DOT1X_MESSAGE_LOGON_PACP,
  /** Changes of Logon/MKA communication variables. */
  DOT1X_MESSAGE_LOGON_MKA,
  /** Changes of Logon/CP communication variables. */
  DOT1X_MESSAGE_LOGON_CP,
  /** Changes of PACP/EAPOL communication variables. */
  DOT1X_MESSAGE_PACP_EAPOL,
  /** Changes of PACP/EAP communication variables. */
  DOT1X_MESSAGE_PACP_EAP,
  /** Changes of MKA/CP communication variables. */
  DOT1X_MESSAGE_MKA_CP,
  /** Changes of MKA/SecY communication variables. */
  DOT1X_MESSAGE_MKA_SECY,
  /** Changes of CP/SecY communication variables. */
  DOT1X_MESSAGE_CP_SECY,
  /** Changes of CP/client communication variables. */
  DOT1X_MESSAGE_CP_CLIENT,
  DOT1X_MESSAGE_LAST
} Dot1xMessage;

#endif /* _DOT1XPAE_TYPES_H */
