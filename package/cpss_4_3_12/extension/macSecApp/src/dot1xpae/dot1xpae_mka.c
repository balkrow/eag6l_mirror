/**

   IEEE 802.1X-2010 MKA protocol.

   File: dot1xpae_mka.c

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#include "dot1xpae_mka.h"
#include "dot1xpae_mkpdu.h"
#include "dot1xpae_eapol.h"
#include "dot1xpae_logon.h"
#include "dot1xpae_netann.h"
#include "dot1xpae_anpdu.h"
#include "dot1xpae_util.h"
#include "macseclmi.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef enum {
  MKA_INITIAL,
  MKA_FAILED,
  MKA_AUTHENTICATED,
  MKA_SECURED
} MkaState;

typedef struct {
  Dot1xMka *mka;
  struct {
    unsigned char buf[32];
    unsigned len;
  } nonce;
} SakDerivationArg;

/*
 * Run a function on every peer station associated with a port
 * (multiple peers if the port is the real port of an interface and a
 * single peer if the port is a virtual port).
 */
static void
foreach_peer(Dot1xPort *port, void (*func)(Dot1xPeer *peer));

/*
 * Advance MKA timers of a real or virtual port.
 */
static void
tick_port(Dot1xPort *port);

/*
 * Advance MKA instance timers.
 */
static void
tick_instance(Dot1xMka *mka);

/*
 * Run pending MKA operations of a real or virtual port.
 */
static void
step_port(Dot1xPort *port);

/*
 * Run MKA instance pending operations.
 */
static void
step_instance(Dot1xMka *mka);

/*
 * Send MKPDUs if needed.
 */
static void
send_instance(Dot1xMka *mka);

/*
 * Run peer pending operations.
 */
static void
step_peer(Dot1xPeer *peer);

/*
 * Advance MKA peer timers.
 */
static void
tick_mkap(Dot1xMkaPeer *mkap);

/*
 * Run MKA peer pending operations.
 */
static void
step_mkap(Dot1xMkaPeer *mkap);

/*
 * Set the MKA state of a real or virtual port.
 */
static void
set_state(Dot1xPort *port, MkaState state);

static void
set_can_suspend(Dot1xPort *port, unsigned value);

/*
 * Send a MKA PDU.
 */
static void
send_pdu(Dot1xMka *mka);

/*
 * Generate a new MKA PDU.
 */
static void
generate_pdu(Dot1xMka *mka, Dot1xMkpdu *pdu);

/*
 * Process pending MKA PDU, if any.
 */
static void
process_pdu(Dot1xMka *mka, const Dot1xMkpdu *pdu);

static void
update_chgd_server(Dot1xMka *mka);

static void
change_mka_key_server(Dot1xMka *mka);

static void
trigger_mkpdu(Dot1xMka *mka)
{
    mka->send_mkpdu = 1;
}

/*
 * Calculate an MKA PDU ICV.
 */
static int
calculate_icv(
  Dot1xMka *mka, Dot1xIcv *icv,
  const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *msdu_buf, unsigned msdu_len);

/*
 * Create a new MKA peer entry corresponding to a member identifier.
 */
static Dot1xMkaPeer *
create_mkap(Dot1xMka *mka, const Dot1xMemberIdentifier *mi);

/*
 * Destroy an MKA peer entry.
 */
static void
destroy_mkap(Dot1xMkaPeer *mkap);

/*
 * Make an MKA peer with the given SCI live with the given effective
 * key server priority (255 = can't be key server). Can be called
 * multiple times. Return 1 if successful.
 */
static int
make_live(Dot1xMkaPeer *mkap, const MacsecSci *sci, unsigned priority);

/*
 * Make an MKA peer potential. Can be called multiple times.
 */
static void
make_not_live(Dot1xMkaPeer *mkap);

/*
 * Mark the MKA peer as desiring MACsec protection.
 */
static void
macsec_on(Dot1xMkaPeer *mkap);

/*
 * Mark the MKA peer as not requiring MACsec protection.
 */
static void
macsec_off(Dot1xMkaPeer *mkap);

/*
 * Set on the receiving status of a peer. Can be called multiple
 * times.
 */
static void
receive_on(Dot1xPeer *peer);

/*
 * Set off the receiving status of a peer. Can be called multiple
 * times.
 */
static void
receive_off(Dot1xPeer *peer);

/*
 * Clear the receive LLPN information of a peer.
 */
static void
clear_llpn(Dot1xPeer *peer);

/*
 * Set on the transmit SA inUse status of a port. Can be called
 * multiple times.
 */
static void
transmit_sa_in_use_on(Dot1xPort *port);

/*
 * Set off the transmit SA inUse status of a port. Can be called
 * multiple times.
 */
static void
transmit_sa_in_use_off(Dot1xPort *port);

/*
 * Set on the receive SA inUse status of a peer. Can be called
 * multiple times.
 */
static void
receive_sa_in_use_on(Dot1xPeer *peer);

/*
 * Set off the receive SA inUse status of a peer. Can be called
 * multiple times.
 */
static void
receive_sa_in_use_off(Dot1xPeer *peer);

/*
 * Activate an MKA instance as a key server candidate.
 */
static void
add_local_key_source(Dot1xMka *mka, unsigned priority);

/*
 * Deactivate an MKA instance as a key server candidate.
 */
static void
delete_local_key_source(Dot1xMka *mka);

/*
 * Activate an MKA peer as a key server candidate.
 */
static void
add_remote_key_source(Dot1xMkaPeer *mkap, unsigned priority);

/*
 * Deactivate an MKA peer as a key server candidate, if it was active.
 */
static void
delete_remote_key_source(Dot1xMkaPeer *mkap);

/*
 * Update the priority of a local key server candidate.
 */
static void
update_local_key_source(Dot1xMka *mka);

/*
 * Update the priority of a remote key server candidate.
 */
static void
update_remote_key_source(Dot1xMkaPeer *mkap, unsigned priority,
    unsigned live_peer_count);

/*
 * Activate a key server candidate.
 */
static void
add_key_source(
  Dot1xPort *port, Dot1xMkaKeySource *key_source,
  Dot1xMka *mka, Dot1xMkaPeer *mkap,
  unsigned pairwise, unsigned supplicant, unsigned priority,
  const MacsecAddress *address, unsigned portid);

/*
 * Deactivate a key server candidate.
 */
static void
delete_key_source(Dot1xMkaKeySource *key_source);

/*
 * Update the priority of a key server candidate.
 */
static void
update_key_source(Dot1xMkaKeySource *key_source, unsigned priority,
  unsigned live_peer_count);

/*
 * Insert a key source into the appropriate position in the list of
 * key sources of a port.
 */
void
insert_key_source(Dot1xPort *port, Dot1xMkaKeySource *source);

/*
 * Remove a key source from the list of key sources of its port.
 */
void
remove_key_source(Dot1xMkaKeySource *source);

/*
 * Update pointer to the principal MKA instance based on the list of
 * key sources. */
void
set_principal(Dot1xPort *port, Dot1xMka *mka);

/*
 * Return 1 if the MKA instance is the current key server.
 */
static int
is_local_key_server(Dot1xMka *mka);

static int
is_mka_key_server(Dot1xMka *mka);

/*
 * Return 1 if the MKA peer is the current key server.
 */
static int
is_remote_key_server(Dot1xMkaPeer *mkap);

/*
 * Return 1 if the MKA instance or one of its peers is the current
 * key server.
 */
static int
is_principal(Dot1xMka *mka);

/*
 * Return 1 if a local MKA instance is the current key server.
 */
static int
key_server_is_local(Dot1xPort *port);

/*
 * Return the principal MKA instance, i.e. the one corresponding the
 * MKA key source elected the key server, if such exists.
 */
static Dot1xMka *
get_principal(Dot1xPort *port);

/*
 * Switch to a principal actor if any.
 */
static void
change_principal(
  Dot1xPort *port);

/*
 * Switch to a new SAK.
 */
static void
change_sak(
  Dot1xPort *port, MacsecCipherSuite cs, MacsecConfidentialityOffset co,
  const Dot1xMemberIdentifier *mi, uint32_t kn,
  const MacsecKey *sak, unsigned an);

/*
 * Switch to a new distributed group CAK produced by the given CAK, or
 * clear the distributed group CAK. Return pointer to the new
 * distributed CAK, if any.
 */
static Dot1xCak *
change_cak(Dot1xCak *cak, const Dot1xCkn *ckn, const MacsecKey *key);

/*
 * Get reference to a peer entry associated with the given SCI. Return
 * NULL if out of peer entries or if there is an existing peer entry
 * with a matching MAC address but different port identifier.
 */
static Dot1xPeer *
refer_peer(Dot1xPort *port, const MacsecSci *sci, Dot1xMka *mka);

/*
 * Release a reference to a peer entry.
 */
static void
unref_peer(Dot1xPort *port, Dot1xPeer *peer);

/*
 * Configure the transmit SA according to current key server information.
 */
static void
create_transmit_sa(Dot1xPort *port);

/*
 * Configure a receive SA according to current key server information
 * if the peer has a corresponding MACsec-level peer.
 */
static void
create_receive_sa_checked(Dot1xPeer *peer);

/*
 * Configure a receive SA according to current key server information.
 */
static void
create_receive_sa(Dot1xPeer *peer);

/*
 * Enable a receive SA if the peer has a corresponding MACsec-level peer.
 */
static void
enable_receive_sa_checked(Dot1xPeer *peer);

/*
 * Enable a receive SA.
 */
static void
enable_receive_sa(Dot1xPeer *peer);

/*
 * Set interface or real or virtual port communication variables.
 */
static void
set_enabled(Dot1xIntf *intf, int value);
static void
set_all_receiving(Dot1xPort *port, int value);
static void
set_server_transmitting(Dot1xPort *port, int value);
static void
set_using_receive_sas(Dot1xPort *port, int value);
static void
set_using_transmit_sa(Dot1xPort *port, int value);
static void
set_elected_self(Dot1xPort *port, int value);
static void
set_new_sak(Dot1xPort *port, int value);
static void
set_chgd_server(Dot1xPort *port, int value);
static void
set_cipher_suite(Dot1xPort *port, MacsecCipherSuite value);
static void
set_cipher_offset(Dot1xPort *port, MacsecConfidentialityOffset value);

/*
 * Find an entry in an MKA PDU peer list corresponding to a member
 * identifier.
 */
static const Dot1xMkpduPeer *
find_pdup(const Dot1xMkpduPeerList *pl, const Dot1xMemberIdentifier *mi);

/*
 * Test KEK and ICK derivation using test vectors. Return 1 if
 * successful.
 */
static int
test_kek_ick_derivation(void);

/*
 * Test SAK derivation using test vectors. Return 1 if successful.
 */
static int
test_sak_derivation(void);

/*
 * Add CAK/CKN information to CMAC for a KEK or ICK generated from a
 * CAK. The `arg' parameter points to a MKA instance.
 */
static void
context_kek_ick(void *arg, Dot1xCryptoCmac *cmac);

/*
 * Add live peer information to CMAC for a SAK generated from a
 * CAK. The `arg' parameter points to an object of type
 * SakDerivationArg.
 */
static void
context_sak(void *arg, Dot1xCryptoCmac *cmac);

/*
 * PN history utilities.
 */
static void
pn_history_fill(Dot1xPnHistory *pnh, uint64_t pn);
static void
pn_history_update(Dot1xPnHistory *pnh, uint64_t pn);
static uint64_t
pn_history_last(Dot1xPnHistory *pnh);

/*
 * Suspension utilities.
 */
static int
is_suspended(Dot1xPort *port);

static void
step_suspension(Dot1xMka *mka);

/*
 * Value for suspendFor if peer doesn't support in-service upgrades.
 */
#define DOT1X_MKA_IN_SERVICE_UPGRADE_UNSUPPORTED ~0

/*
 * Constants for key derivation.
 */
static const unsigned char zero[16];
static const char *kek_label = "IEEE8021 KEK";
static const char *ick_label = "IEEE8021 ICK";
static const char *sak_label = "IEEE8021 SAK";


/*
 * Public functions.
 */

Dot1xMka *
dot1x_create_mka(Dot1xCak *cak)
{
  Dot1xIntf *intf = cak->intf;
  Dot1xPort *port = cak->port;
  Dot1xMka *mka = NULL;
  Dot1xFormatBuffer fb;

  /* get a new MKA instance */
  if (!(mka = dot1x_mkas_insert(&intf->mkas)))
    {
      dot1x_error(intf, "out of MKA instances");
      goto fail;
    }
  mka->cak = cak;
  dot1x_mka_peers_init(&mka->peers);
  dot1x_sak_pns_init(&mka->sak_pns);

  /* derive KEK and ICK */
  mka->kek.len = cak->key.len;
  dot1x_derive_key(
    intf, mka->kek.buf, mka->kek.len, &cak->key, kek_label,
    context_kek_ick, mka);
  mka->ick.len = cak->key.len;
  dot1x_derive_key(
    intf, mka->ick.buf, mka->ick.len, &cak->key, ick_label,
    context_kek_ick, mka);

  /* init member identifier and message number */
  if (!dot1x_crypto_random_generate(
        &intf->crypto_random, mka->mi.buf, sizeof mka->mi.buf))
    {
      dot1x_error(intf, "RNG failure creating MKA instance");
      goto fail;
    }
  mka->mn = 1;

  dot1x_debug_mka_actor(
    mka, "created with MI %s", dot1x_format_member_identifier(&fb, &mka->mi));

  dot1x_debug_mka_actor(
    mka, "is associated with %s", dot1x_format_cak(&fb, mka->cak));

  add_local_key_source(mka, intf->ctrl.actor_priority);

  /* start timer for signalling MKA failed if appropriate */
  if (!port->mka_authenticated && !port->mka_secured)
    {
      port->mka_fail_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
      set_state(port, MKA_INITIAL);
    }

  /* note a distributed CAK obtained from cache */
  if (cak->type == DOT1X_CAK_DISTRIBUTED && !port->distributed_cak)
    port->distributed_cak = cak;

  return mka;

 fail:
  dot1x_destroy_mka(mka);
  return NULL;
}

void
dot1x_destroy_mka(Dot1xMka *mka)
{
  Dot1xIntf *intf;
  Dot1xPort *port;
  if (!mka)
    return;

  port = mka->cak->port;
  intf = port->intf;

  dot1x_debug_mka_actor(mka, "being destroyed");

  /* destroy MKA peers */
  dot1x_mka_peers_foreach(&mka->peers, destroy_mkap);

  /* stop being a potential key server */
  if (mka->key_source.mka)
    delete_local_key_source(mka);

  dot1x_mkas_remove(&intf->mkas, mka);
  memset(mka, 0, sizeof *mka);

  if (port->principal == mka)
    change_principal(port);
}

void
dot1x_tick_mka(Dot1xIntf *intf)
{
  dot1x_mkas_foreach(&intf->mkas, tick_instance);
  dot1x_foreach_port(intf, tick_port);
}

int
dot1x_step_mka(Dot1xIntf *intf)
{
  /* pass through the enable control to the Logon process input variable */
  set_enabled(intf, intf->ctrl.mka_enable);

  dot1x_mkas_foreach(&intf->mkas, step_instance);
  /* run all MKA instances */

  /* run all peers */
  dot1x_peers_foreach(&intf->peers, step_peer);

  /* run all real or virtual ports */
  dot1x_foreach_port(intf, step_port);

  if (intf->mka_changed)
    {
      intf->mka_changed = 0;
      return 1;
    }

  return 0;
}

void
dot1x_send_mka(Dot1xIntf *intf)
{
    dot1x_mkas_foreach(&intf->mkas, send_instance);
}

void
dot1x_create_sas(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;
  Dot1xMemberIdentifier *mi;
  uint32_t kn;
  MacsecSalt saltbuf, *salt;
  Dot1xFormatBuffer fb;

  /* skip virtual port that has no MACsec-level virtual port */
  if (port != &intf->rport && port->peer_count <= 0)
    return;

  if (port->distributed_ki.kn == 0)
    {
      dot1x_error_port(port, "no valid SAK");
      return;
    }

  switch (port->cipher_suite)
    {
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_256:
      salt = &saltbuf;
      mi = &port->distributed_ki.mi;
      kn = port->distributed_ki.kn;
      assert(sizeof salt->buf == sizeof mi->buf);
      memcpy(salt->buf, mi->buf, sizeof salt->buf);
      salt->buf[0] ^= (kn >> 8) & 0xff;
      salt->buf[1] ^= kn & 0xff;
      salt->buf[2] ^= (kn >> 24) & 0xff;
      salt->buf[3] ^= (kn >> 16) & 0xff;
      break;
    default:
      salt = NULL;
      break;
    }

  dot1x_debug_mka_secy(
    port, "installKey(sak, %s)",
    dot1x_format_key_identifier(&fb, &port->distributed_ki));

  macsec_install_key(
    port->intf->macsec_intf, port->portid,
    &port->distributed_sak, (void *)&port->distributed_ki, salt);

  memcpy(&port->macsec_ki, &port->distributed_ki, sizeof port->macsec_ki);

  foreach_peer(port, create_receive_sa_checked);
  create_transmit_sa(port);
}

void
dot1x_delete_sas(Dot1xPort *port)
{
  (void)port;
}

void
dot1x_enable_receive_sas(Dot1xPort *port)
{
  foreach_peer(port, enable_receive_sa_checked);
  port->receive_sas_enabled = 1;
}

void
dot1x_enable_transmit_sa(Dot1xPort *port)
{
  transmit_sa_in_use_off(port);

  /* skip virtual port that has no MACsec-level virtual port */
  if (port != &port->intf->rport && port->peer_count <= 0)
    return;

  dot1x_debug_mka_secy(port, "TransmitSA.enableTransmit");
  macsec_enable_transmit(
    port->intf->macsec_intf, port->portid, port->new_transmit_sa_an);

  /* assume immediate inUse = TRUE */
  transmit_sa_in_use_on(port);
}

void
dot1x_receive_mkpdu(
  Dot1xIntf *intf,
  const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *msdu_buf, unsigned msdu_len)
{
  const unsigned char *body_buf = msdu_buf + DOT1X_EAPOL_HEADER_SIZE;
  unsigned body_len = msdu_len - DOT1X_EAPOL_HEADER_SIZE;
  Dot1xMkpdu *pdu = &intf->mkpdu;
  Dot1xAnpdu *ann = &intf->anpdu;
  Dot1xFormatBuffer fb1, fb2;
  Dot1xCak *cak;
  Dot1xMka *mka, *replaced_mka;
  Dot1xIcv icv;

  if (!intf->ctrl.mka_enable)
    {
      intf->stat.eapol_mk_no_ckn_rx++;
      return;
    }

  /* decode MKA PDU */
  pdu->announcement = ann;
  if (!dot1x_decode_mkpdu(intf, pdu, body_buf, body_len))
    {
      dot1x_debug_discard(intf, "cannot decode MKA PDU");
      return;
    }

  /* find a matching CAK */
  if (!(cak = dot1x_caks_lookup(&intf->caks, &pdu->ckn)))
    {
      intf->stat.eapol_mk_no_ckn_rx++;
      dot1x_debug_discard(
        intf, "MKA PDU with unknown CAK %s",
        dot1x_format_ckn(&fb1, &pdu->ckn));
      return;
    }

  /* ensure the CAK has a running MKA instance */
  if (!(mka = cak->mka))
    {
      dot1x_debug_discard(
        intf, "MKA PDU with inactive CAK %s",
        dot1x_format_ckn(&fb1, &pdu->ckn));

      /* revive a cached CAK */
      dot1x_refresh_cak(cak);
      return;
    }
  replaced_mka = cak->replaced_mka;

  /* authenticate */
  if (!(calculate_icv(mka, &icv, dst, src, msdu_buf, msdu_len)))
    return;

  if (memcmp(&icv, &pdu->icv, sizeof icv))
    {
      intf->stat.eapol_mk_invalid_rx++;
      dot1x_debug_discard(
        intf, "MKA PDU with CAK %s failed ICV check",
        dot1x_format_ckn(&fb1, &pdu->ckn));
      return;
    }

  dot1x_debug_mka(
    mka, "received MKPDU, SCI %s, MI %s, MN %lu",
    dot1x_format_sci(&fb1, &pdu->sci),
    dot1x_format_member_identifier(&fb2, &pdu->mi),
    (unsigned long)pdu->mn);

  if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_MKA) &&
      dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
    dot1x_print_mkpdu(intf, pdu);

  process_pdu(mka, pdu);
  if (replaced_mka)
    process_pdu(replaced_mka, pdu);
  intf->changed = 1;
}

Dot1xMkaKeySource *
dot1x_get_key_server(const Dot1xPort *port)
{
  Dot1xMkaKeySource *source = NULL;

  if (port->principal)
    source = port->principal->key_server;

  return source;
}

int
dot1x_test_mka(void)
{
  int ok = 1;

  ok &= test_kek_ick_derivation();
  ok &= test_sak_derivation();
  return ok;
}

/*
 * Static functions.
 */

static void
foreach_peer(Dot1xPort *port, void (*func)(Dot1xPeer *peer))
{
  Dot1xIntf *intf = port->intf;

  if (port == &intf->rport)
    dot1x_peers_foreach(&intf->peers, func);
  else
    func((void *)((unsigned char *)port - offsetof(Dot1xPeer, vport)));
}

static void
tick_port(Dot1xPort *port)
{
  unsigned an;
  uint64_t next_pn;

  if (port->mka_fail_ticks > 1)
    port->mka_fail_ticks--;

  if (port->sak_hold_ticks)
    port->sak_hold_ticks--;

  if (port->suspended_while_ticks)
    port->suspended_while_ticks--;

  if (port->local_suspended_while_ticks)
    port->local_suspended_while_ticks--;

  /* skip virtual port that has no MACsec-level virtual port */
  if (port != &port->intf->rport && port->peer_count <= 0)
    return;

  an = port->transmit_sa_an;

  macsec_get_transmit_sa_next_pn(
    port->intf->macsec_intf, port->portid, an, &next_pn);

  pn_history_update(&port->transmit_sa_next_pn_history[an], next_pn);
}

static void
tick_instance(Dot1xMka *mka)
{
  dot1x_mka_peers_foreach(&mka->peers, tick_mkap);

  if (mka->send_ticks)
    mka->send_ticks--;

  if (mka->keyserver_suspend_ticks)
    mka->keyserver_suspend_ticks--;
}

static void
step_port(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;
  Dot1xMka *principal;
  Dot1xNetwork *netw = NULL;

  /* clear group CAK if required by management */
  if (port->distributed_cak && !intf->ctrl.group_ca_enable)
    {
      dot1x_debug_cak_port(port, "clearing group CAK");
      change_cak(port->distributed_cak, NULL, NULL);
    }

  principal = get_principal(port);
  if (principal && !(netw = principal->cak->network))
    principal = NULL;

  /* update MKA output state */
  if (!intf->ctrl.mka_enable)
    {
      set_state(port, MKA_INITIAL);
    }
  else
  if ((!port->mka_authenticated && !port->mka_secured &&
       port->mka_fail_ticks == 1) ||
      ((port->mka_authenticated || port->mka_secured) &&
       (!principal || port->peer_count <= 0)))
    {
      port->mka_fail_ticks = 0;
      set_state(port, MKA_FAILED);
    }
  else
  if (principal && principal->live_peer_count > 0)
    {
      port->mka_fail_ticks = 0;
      if (principal->macsec_peer_count <= 0 &&
          netw->ctrl.unsecured_allowed == DOT1X_UNSECURED_ALLOWED_MKA_SERVER)
        set_state(port, MKA_AUTHENTICATED);
      else
        set_state(port, MKA_SECURED);
    }

  /* consume newInfo */
  if (port->new_info)
    {
      port->new_info = 0;
      dot1x_debug_mka_cp(port, "newInfo = %s", dot1x_bool(port->new_info));
      if (principal)
        trigger_mkpdu(principal);
    }

  /* update allReceiving */
  if (port->peer_count > 0 && port->receiving_peer_count >= port->peer_count)
    set_all_receiving(port, 1);
  else
    set_all_receiving(port, 0);

  /* update usingReceiveSAs */
  if (port->peer_count > 0 && port->using_peer_count >= port->peer_count)
    set_using_receive_sas(port, 1);
  else
    set_using_receive_sas(port, 0);

  /* update usingTransmitSA */
  set_using_transmit_sa(port, port->transmit_sa_in_use);
}

static void
step_instance(Dot1xMka *mka)
{
  Dot1xPort *port = mka->cak->port;
  Dot1xIntf *intf = port->intf;
  MacsecCipherSuite cs = intf->ctrl.cipher_suite;
  MacsecConfidentialityOffset co = intf->ctrl.confidentiality_offset;
  Dot1xCak *new_cak;
  MacsecKey sak;
  Dot1xCkn ckn;
  MacsecKey cak;
  SakDerivationArg sda;
  unsigned distributed_cak_needed;

  /* do nothing if network removed */
  if (!mka->cak->network)
    return;

  /* request replacement actor if MN or KN exhaustion approaching */
  if (!mka->replace)
    {
      if (mka->mn >= 0xffffff00)
        {
          dot1x_debug_mka_actor(mka, "near MN exhaustion");
          mka->replace = 1;
        }
      if (mka->kn >= 0xffffff00)
        {
          dot1x_debug_mka_actor(mka, "near KN exhaustion");
          mka->replace = 1;
        }
    }

  dot1x_mka_peers_foreach(&mka->peers, step_mkap);

  change_mka_key_server(mka);

  if (is_principal(mka))
    step_suspension(mka);

  if (is_mka_key_server(mka))
    {
      /* determine whether or not there should be a distributed CAK */
      distributed_cak_needed =
        /* group CAs are not prohibited and */
        intf->ctrl.group_ca_enable &&
        /* either this CAK is the distributed CAK or */
        (mka->cak == port->distributed_cak ||
         /* this CAK is pairwise and port has seen multiple peers */
         (mka->cak->pairwise && port->highest_peer_count > 1));

      /* generate/destroy group CAK if necessary */
      if (!port->distributed_cak && distributed_cak_needed)
        {
          dot1x_debug_cak_port(port, "generating new group CAK");

          /* generate 16-byte null-padded CKN and a 16-byte CAK */
          memset(&ckn, 0, sizeof ckn);
          if (dot1x_crypto_random_generate(
                &intf->crypto_random, ckn.buf, 16) &&
              dot1x_crypto_random_generate(
                &intf->crypto_random, cak.buf, sizeof cak.buf))
            {
              ckn.len = 16;
              cak.len = 16;
              if ((new_cak = change_cak(mka->cak, &ckn, &cak)))
                new_cak->generated = 1;
            }
          else
            {
              dot1x_error(intf, "RNG failure generating group CAK");
            }
        }
      else if (port->distributed_cak && !distributed_cak_needed)
        {
          dot1x_debug_cak_port(port, "clearing group CAK");
          change_cak(port->distributed_cak, NULL, NULL);
          return;
        }

      /* generate new SAK if: */
      if (is_principal(mka) &&
          !is_suspended(port) &&
          /* there is no valid SAK, the set of live peers has changed,
             PNs are being exhausted or cipher parameters have
             changed, the last distributed CAK was created by some
             other MKA and */
          (!port->distributed_ki.kn ||
           mka->live_peers_changed ||
           mka->pn_exhaust_pending ||
           cs != port->cipher_suite ||
           co != port->cipher_offset ||
           memcmp(&port->distributed_ki.mi, &mka->mi, sizeof mka->mi)) &&

          /* a CAK is not being distributed or this MKA is based on it, and */
          (!distributed_cak_needed || mka->cak == port->distributed_cak) &&

          /* there are live peers, and */
          mka->live_peer_count > 0 &&

          /* potential peer list is empty, or at least MKA life time since any
             prior SAK generation */
          (mka->live_peer_count >= mka->peer_count || !port->sak_hold_ticks))
        {
          mka->live_peers_changed = 0;
          mka->pn_exhaust_pending = 0;
          port->sak_hold_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;

          switch (cs)
            {
            case MACSEC_CIPHER_SUITE_GCM_AES_128:
            case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
              sak.len = 16;
              break;
            default:
              sak.len = 32;
              break;
            }

          dot1x_debug_sak(port, "generating new %u-bit SAK", sak.len << 3);

          sda.mka = mka;
          sda.nonce.len = sak.len;
          if (!dot1x_crypto_random_generate(
                &intf->crypto_random, sda.nonce.buf, sda.nonce.len))
            {
              dot1x_error(intf, "RNG failure generating SAK");
              change_sak(port, cs, co, NULL, 0, NULL, 0);
            }
          else
            {
              mka->kn++;
              dot1x_derive_key(
                intf, sak.buf, sak.len, &mka->cak->key, sak_label,
                context_sak, &sda);

              update_chgd_server(mka);

              change_sak(port, cs, co, &mka->mi, mka->kn, &sak, port->next_an);
              port->distribute_sak = 1;
            }
          intf->mka_changed = 1;
          trigger_mkpdu(mka);
        }
    }

  /* CAK deactivation timer off if this is the principal instance and */
  if ((is_principal(mka) &&
      /* there are live peers or */
      (mka->live_peer_count > 0 ||
       /* the CAK is a pre-shared CAK (pairwise or group ) OR */
       mka->cak->type == DOT1X_CAK_PAIRWISE ||
       mka->cak->type == DOT1X_CAK_GROUP)) ||
      /* the CAK is preshared */
      mka->cak->preshared ||
      is_suspended(port))
    {
      if (mka->cak->active_ticks)
        mka->cak->active_ticks = 0;
    }
  else
    {
      if (!mka->cak->active_ticks)
        mka->cak->active_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
    }

  /* control sending of KMD in MKPDUs of pairwise CAs */
  if (intf->ctrl.cak_caching_enable &&
      !mka->cak->preshared && mka->cak->authenticator &&
      key_server_is_local(port))
    {
      if (!mka->kmd_count)
        mka->kmd_count = 4;
    }
  else
    {
      if (mka->kmd_count)
        mka->kmd_count = 0;
    }

  if (mka->send_ticks == 0)
    {
        trigger_mkpdu(mka);
    }
}

static void
send_instance(Dot1xMka *mka)
{
  Dot1xPort *port = mka->cak->port;
  Dot1xIntf *intf = port->intf;

  if (mka->send_mkpdu != 0)
    {
      mka->send_mkpdu = 0;

      if (intf->ctrl.bounded_delay)
        mka->send_ticks =
          DOT1X_MKA_BOUNDED_HELLO_MILLISECONDS * DOT1X_TICKS_PER_SECOND / 1000;
      else
        mka->send_ticks =
          DOT1X_MKA_HELLO_TIME * DOT1X_TICKS_PER_SECOND;
      send_pdu(mka);
    }
}

static void
step_peer(Dot1xPeer *peer)
{
  Dot1xPort *port = peer->port;
  uint64_t lowest_pn;

  /* update receive SA according to recent lowest PN report from the peer */
  if (peer->llpn_changed)
    {
      peer->llpn_changed = 0;

      lowest_pn = peer->llpn;
      if (lowest_pn < 1)
        lowest_pn = 1;

      if (peer->macsec_peer)
        {
          dot1x_debug_mka_secy(
            port, "ReceiveSA.updtLowestPN = %lu", (unsigned long)lowest_pn);

          macsec_set_receive_sa_lowest_pn(
            peer->macsec_peer, port->lan, lowest_pn);
        }
    }
}

static void
tick_mkap(Dot1xMkaPeer *mkap)
{
  if (mkap->potential_ticks)
    mkap->potential_ticks--;

  if (mkap->live_ticks)
    mkap->live_ticks--;
}

static void
step_mkap(Dot1xMkaPeer *mkap)
{
  Dot1xMka *mka = mkap->mka;

  if (mkap->live_ticks <= 0 && mkap->potential_ticks <= 0)
    {
      destroy_mkap(mkap);
      mka->cak->port->intf->mka_changed = 1;
    }
  else if (mkap->live && mkap->live_ticks <= 0)
    {
      make_not_live(mkap);
    }
}

static void
set_state(Dot1xPort *port, MkaState state)
{
  Dot1xIntf *intf = port->intf;
  int failed, authenticated, secured;

  if (is_suspended(port) && port->mka_secured)
    return;

  switch (state)
    {
    case MKA_INITIAL:
    default:
      failed = 0;
      authenticated = 0;
      secured = 0;
      break;

    case MKA_FAILED:
      failed = 1;
      authenticated = 0;
      secured = 0;
      break;

    case MKA_AUTHENTICATED:
      failed = 0;
      authenticated = 1;
      secured = 0;
      break;

    case MKA_SECURED:
      failed = 0;
      authenticated = 0;
      secured = 1;
      break;
    }

  if (failed != port->mka_failed)
    {
      port->mka_failed = failed;
      dot1x_debug_logon_mka(
        port, "MKA.failed = %s", dot1x_bool(port->mka_failed));
      intf->mka_changed = 1;
    }

  if (authenticated != port->mka_authenticated)
    {
      port->mka_authenticated = authenticated;
      dot1x_debug_logon_mka(
        port, "MKA.authenticated = %s", dot1x_bool(port->mka_authenticated));
      intf->mka_changed = 1;
    }

  if (secured != port->mka_secured)
    {
      port->mka_secured = secured;
      dot1x_debug_logon_mka(
        port, "MKA.secured = %s", dot1x_bool(port->mka_secured));

      if (!port->mka_secured)
        {
          dot1x_debug_sak(
            port, "clearing SAK after leaving the secured state");
          change_sak(
            port, intf->ctrl.cipher_suite, intf->ctrl.confidentiality_offset,
            NULL, 0, NULL, 0);
        }

      intf->mka_changed = 1;
    }
}

static void
send_pdu(Dot1xMka *mka)
{
  Dot1xPort *port = mka->cak->port;
  Dot1xIntf *intf = port->intf;
  const MacsecAddress *dst, *src;
  Dot1xFormatBuffer fb1, fb2;
  Dot1xMkpdu *pdu = &intf->mkpdu;
  Dot1xAnpdu *ann = &intf->anpdu;
  unsigned char *msdu_buf = intf->msdu;
  unsigned msdu_max = sizeof intf->msdu, msdu_len;
  unsigned char *body_buf = msdu_buf + DOT1X_EAPOL_HEADER_SIZE;
  unsigned body_max = msdu_max - DOT1X_EAPOL_HEADER_SIZE, body_len;

  pdu->announcement = ann;
  generate_pdu(mka, pdu);

  if (!dot1x_encode_mkpdu(intf, pdu, body_buf, body_max, &body_len))
    return;

  dot1x_encode_eapol_header(msdu_buf, 3, DOT1X_EAPOL_MKA, body_len);
  msdu_len = body_len + DOT1X_EAPOL_HEADER_SIZE;

  dst = dot1x_eapol_address(intf->ctrl.eapol_group);
  src = &intf->local_address;

  if (!(calculate_icv(mka, &pdu->icv, dst, src, msdu_buf, msdu_len)))
    return;

  memcpy(msdu_buf + msdu_len - sizeof pdu->icv, &pdu->icv, sizeof pdu->icv);

  dot1x_debug_mka(
    mka, "sending MKPDU, SCI %s, MI %s, MN %lu",
    dot1x_format_sci(&fb1, &pdu->sci),
    dot1x_format_member_identifier(&fb2, &pdu->mi),
    (unsigned long)pdu->mn);

  if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_MKA) &&
      dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
    dot1x_print_mkpdu(intf, pdu);

  dot1x_debug_eapol(
    intf, "sending EAPOL-MKA packet to %s", dot1x_format_address(&fb1, dst));

  if (!macsec_send_uncontrolled(
        intf->macsec_intf, dst, src, msdu_buf, msdu_len))
    dot1x_error(intf, "cannot send to uncontrolled port");
}

static void
generate_pdu(Dot1xMka *mka, Dot1xMkpdu *pdu)
{
  Dot1xCak *cak = mka->cak;
  Dot1xPort *port = cak->port;
  Dot1xIntf *intf = port->intf;
  Dot1xNetwork *netw = cak->network;
  Dot1xMkaPeer *mkap;
  Dot1xMkpduPeerList *pl;
  Dot1xMkpduPeer *p;
  Dot1xAnpdu *ann;
  int xpn;

  switch (port->cipher_suite)
    {
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_256:
      xpn = 1;
      break;
    default:
      xpn = 0;
      break;
    }

  ann = pdu->announcement;
  memset(pdu, 0, sizeof *pdu);
  pdu->announcement = ann;

  /* basic parameter set */
  pdu->version = 2;
  /* Key server flag needs to be set for local key server instance as well as for any
     other local MKA instances in case of PSK or EAP pairwise CAK */
  if (is_mka_key_server(mka) ||
      (mka == cak->replaced_mka && is_mka_key_server(cak->mka)))
    pdu->key_server = 1;
  else
    pdu->key_server = 0;
  pdu->key_server_priority = mka->key_source.priority;
  if (netw->ctrl.unsecured_allowed == DOT1X_UNSECURED_ALLOWED_MKA_SERVER)
    pdu->macsec_desired = 0;
  else
    pdu->macsec_desired = 1;
  pdu->macsec_capability =
    MACSEC_CAPABILITY_CONFIDENTIALITY_WITH_OFFSET;

  pdu->sci.address = intf->local_address;
  pdu->sci.portid = port->portid;
  pdu->mi = mka->mi;
  pdu->mn = mka->mn++;
  pdu->mka_algorithm = DOT1X_MKA_ALGORITHM_8021X_2009;
  pdu->ckn = cak->ckn;

  /* SAK use parameter set */
  if (port->lki.kn != 0 || port->lrx)
    {
      pdu->lan = port->lan;
      pdu->ltx = port->ltx;
      pdu->lrx = port->lrx;
      pdu->lki = port->lki;
      pdu->llpn = pn_history_last(
        &port->transmit_sa_next_pn_history[port->lan]);
      if (pdu->llpn > 0)
        pdu->llpn--;
      if (((!xpn && (uint32_t)pdu->llpn >= 0xc0000000) ||
           (xpn && pdu->llpn >= 0xc000000000000000ULL)) &&
          !mka->pn_exhaust_pending)
        {
          dot1x_debug_mka_actor(mka, "near PN exhaustion");
          mka->pn_exhaust_pending = 1;
        }
    }
  if (port->oki.kn != 0 || port->orx)
    {
      pdu->oan = port->oan;
      pdu->otx = port->otx;
      pdu->orx = port->orx;
      pdu->oki = port->oki;
      pdu->olpn = pn_history_last(
        &port->transmit_sa_next_pn_history[port->oan]);
      if (pdu->olpn > 0)
        pdu->olpn--;
    }
  if (port->lki.kn != 0 || port->oki.kn != 0 || port->lrx || port->orx)
    {
      if (!port->protect_frames)
        pdu->ptx = 1;
      if (port->validate_frames != MACSEC_VALIDATE_FRAMES_STRICT)
        pdu->prx = 1;
      if (intf->ctrl.bounded_delay)
        pdu->dp = 1;
      pdu->has_sak_use = 1;
    }

  /* distributed SAK parameter set */
  /* add if distributed CAK is not going to be added */
  if (pdu->key_server &&
      is_local_key_server(mka) &&
      port->distribute_sak &&
      (!cak->pairwise || !port->distributed_cak))
    {
      if (/* in secured mode and have a SAK to distribute */
          port->mka_secured && port->distributed_ki.kn &&
          /* XXX using the SAK ourselves */
          !memcmp(&port->distributed_ki, &port->lki, sizeof port->lki) &&
          /* XXX not all peers have reported receiving with the new key */
          !port->all_receiving &&
          /* the SAK is generated in the context of this MKA */
          !memcmp(&port->distributed_ki.mi, &mka->mi, sizeof mka->mi))
        {
          if (port->cipher_suite != MACSEC_CIPHER_SUITE_GCM_AES_128)
            {
              pdu->distributed_cs = port->cipher_suite;
              pdu->has_distributed_cs = 1;
            }
          pdu->distributed_co = port->cipher_offset;
          pdu->distributed_an = port->distributed_an;
          pdu->distributed_kn = port->distributed_ki.kn;
          dot1x_crypto_wrap(
            &pdu->distributed_sak, &mka->kek, &port->distributed_sak);
          pdu->has_distributed_sak = 1;
        }
      else if (port->mka_authenticated)
        {
          pdu->has_distributed_plaintext = 1;
          pdu->has_distributed_sak = 1;
        }
    }

  /* distributed CAK parameter set */
  if (pdu->key_server &&
      (cak->pairwise && port->distributed_cak))
    {
      dot1x_ckncpy(&pdu->distributed_ckn, &port->distributed_cak->ckn);
      dot1x_crypto_wrap(
        &pdu->distributed_cak, &mka->kek, &port->distributed_cak->key);
      pdu->has_distributed_cak = 1;
    }

  if (pdu->has_distributed_cak || pdu->has_distributed_sak)
    mka->cak->sent_or_received_cak_or_sak = 1;

  /* KMD */
  if (intf->ctrl.cak_caching_enable && netw->ctrl.kmd.len &&
      (mka->kmd_count > 1 || cak->generated))
    {
      mka->kmd_count--;
      dot1x_kmdcpy(&pdu->kmd, &netw->ctrl.kmd);
      pdu->has_kmd = 1;
    }

  /* announcement */
  if (intf->ctrl.announcer_enable)
    {
      dot1x_generate_announcement(port, pdu->announcement);
      pdu->has_announcement = 1;
    }

  /* XPN */
  pdu->has_xpn = 1;
  if (pdu->key_server)
    {
      pdu->xpn_mst = port->suspended_while;
    }
  else
    {
      unsigned mst = port->suspend_for;

      if (mst == 0 && port->resuming_suspend)
        mst = dot1x_to_secs(port->local_suspended_while_ticks);

      pdu->xpn_mst = mst;
    }

  if (xpn)
    {
      pdu->xpn_llpn = pdu->llpn >> 32;
      pdu->xpn_olpn = pdu->olpn >> 32;
    }
  else
    {
      pdu->xpn_llpn = 0;
      pdu->xpn_olpn = 0;
    }

  /* live and potential peer lists */
  mkap = NULL;
  while ((mkap = dot1x_mka_peers_next(&mka->peers, mkap)))
    {
      if (mkap->live)
        pl = &pdu->lpl;
      else
        pl = &pdu->ppl;

      if (pl->num >= sizeof pl->tab / sizeof pl->tab[0])
        {
          dot1x_error_mka(
            mka, "has too many %s peers", mkap->live ? "live" : "potential");
          pl->num = 0;
          break;
        }

      p = &pl->tab[pl->num++];
      memcpy(&p->mi, &mkap->mi, sizeof p->mi);
      p->mn = mkap->mn;
    }
}

static void
process_pdu(Dot1xMka *mka, const Dot1xMkpdu *pdu)
{
  Dot1xPort *port = mka->cak->port;
  Dot1xIntf *intf = port->intf;
  Dot1xMkaPeer *sender, *mkap;
  const Dot1xMkpduPeer *pdup;
  int sender_live, mutually_live;
  uint32_t reflected_mn;
  MacsecCipherSuite cs;
  MacsecConfidentialityOffset co;
  unsigned an, ltx, lrx, priority, suspend_for;
  uint64_t llpn;
  uint32_t kn;
  MacsecKey sak;
  MacsecKey cak;
  int xpn;
  int i;

  switch (port->cipher_suite)
    {
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_256:
      xpn = 1;
      break;
    default:
      xpn = 0;
      break;
    }

  /* check version */
  if (pdu->version < 1)
    {
      dot1x_debug_discard_mka(
        mka, "received PDU with invalid MKA version %u", pdu->version);
      return;
    }

  /* check for duplicate MAC address */
  if (!memcmp(&pdu->sci, &intf->local_address, sizeof intf->local_address))
    {
      dot1x_debug_discard_mka(mka, "received PDU with duplicate MAC address");
      mka->duplicate = 1;
      return;
    }

  /* check for duplicate member identifier */
  if (!memcmp(&pdu->mi, &mka->mi, sizeof pdu->mi))
    {
      dot1x_debug_discard_mka(
        mka, "received PDU with duplicate member identifier");
      mka->duplicate = 1;
      return;
    }

  /* find potential existing MKA peer entry */
  sender = dot1x_mka_peers_lookup(&mka->peers, &pdu->mi);

  /* verify message number */
  if (sender &&
      pdu->mn <= sender->mn)
    {
      dot1x_debug_discard_mka(mka, "received duplicate or delayed PDU");
      return;
    }

  /* get the sender's perception of us */
  sender_live = 0;
  mutually_live = 0;
  if ((pdup = find_pdup(&pdu->lpl, (void *)&mka->mi)))
    {
      sender_live = 1;
      mutually_live = 1;
      reflected_mn = pdup->mn;
    }
  else if ((pdup = find_pdup(&pdu->ppl, (void *)&mka->mi)))
    {
      sender_live = 1;
      reflected_mn = pdup->mn;
    }

  /* another check for duplicate member identifier */
  if (sender_live && reflected_mn >= mka->mn)
    {
      dot1x_debug_discard_mka(mka, "received PDU with future message number");
      mka->duplicate = 1;
      return;
    }

  /* check for delayed message */
  if (sender_live && mka->mn > 3 && reflected_mn < mka->mn - 3)
    {
      dot1x_debug_discard_mka(mka, "received PDU with reflected MN too old");
      return;
    }

  /* create peer record for the sender if not already done */
  if (!sender &&
      !(sender = create_mkap(mka, &pdu->mi)))
    return;

  /* update and refresh sender's peer record */
  sender->mn = pdu->mn;
  sender->potential_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;

  if (sender_live)
    sender->live_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;

  /* determine effective key server priority */
  priority = pdu->key_server_priority;
  sender->key_source.willing_key_server = pdu->key_server;

  /* make sender live iff it knows about us and if that fails stop
     here */
  if (sender_live)
    {
      if (!make_live(sender, &pdu->sci, priority))
        return;
    }
  else
    {
      make_not_live(sender);
    }

  /* handle MACsec protection flags */
  if (pdu->macsec_desired && pdu->macsec_capability > 0)
    macsec_on(sender);
  else
    macsec_off(sender);

  /* handle changed key server priority */
  if (sender_live)
    update_remote_key_source(sender, priority, pdu->lpl.num);

  /* handle distributed CAK if the sender is the key server */
  if (mutually_live &&
      !mka->replace &&
      is_remote_key_server(sender) &&
      pdu->has_distributed_cak &&
      intf->ctrl.group_ca_enable)
    {
      if (port->distributed_cak_source != sender->peer ||
          !port->distributed_cak ||
          dot1x_ckncmp(&port->distributed_cak->ckn, &pdu->distributed_ckn))
        {
          port->distributed_cak_source = sender->peer;
          dot1x_crypto_unwrap(&pdu->distributed_cak, &mka->kek, &cak);
          change_cak(mka->cak, &pdu->distributed_ckn, &cak);
          mka->cak->sent_or_received_cak_or_sak = 1;
        }
    }

  /* handle distributed SAK if the sender is the key server */
  if (mutually_live &&
      !mka->replace &&
      is_remote_key_server(sender) &&
      pdu->has_distributed_sak)
    {
      if (!pdu->has_distributed_plaintext)
        {
          if (pdu->has_distributed_cs)
            cs = pdu->distributed_cs;
          else
            cs = MACSEC_CIPHER_SUITE_GCM_AES_128;
          co = pdu->distributed_co;
          an = pdu->distributed_an;
          kn = pdu->distributed_kn;
          dot1x_crypto_unwrap(&pdu->distributed_sak, &mka->kek, &sak);
        }
      else
        {
          cs = MACSEC_CIPHER_SUITE_GCM_AES_128;
          co = MACSEC_CONFIDENTIALITY_NOT_USED;
          an = 0;
          kn = 0;
          memset(&sak, 0, sizeof sak);
        }

      if (port->cipher_suite != cs ||
          port->cipher_offset != co ||
          an != port->distributed_an ||
          kn != port->distributed_ki.kn ||
          (kn &&
           memcmp(&sender->mi, &port->distributed_ki.mi, sizeof sender->mi)))
        {
          if (kn)
            dot1x_debug_sak(port, "received new %u-bit SAK", sak.len << 3);
          else
            dot1x_debug_sak(port, "received empty SAK");

          update_chgd_server(mka);

          set_principal(port, mka);

          change_sak(port, cs, co, &sender->mi, kn, &sak, an);
          mka->cak->sent_or_received_cak_or_sak = 1;
        }
    }

  /* handle SAK use */
  if (pdu->has_sak_use &&
      pdu->lan == port->distributed_an &&
      (is_suspended(port) ||
       !memcmp(&pdu->lki, &port->distributed_ki, sizeof pdu->lki)))
    {
      ltx = pdu->ltx;
      lrx = pdu->lrx;
      if (xpn && pdu->has_xpn)
        {
          llpn = pdu->xpn_llpn;
          llpn = llpn << 32;
          llpn = llpn | pdu->llpn;
        }
      else
        {
          llpn = pdu->llpn;
        }
    }
  else
    {
      ltx = 0;
      lrx = 0;
      llpn = 0;
    }

  /* if the local instance is the princpal one handle sender's PN report */
  if (sender_live && is_principal(mka) &&
      llpn != sender->peer->llpn)
    {
      sender->peer->llpn = llpn;
      sender->peer->llpn_changed = 1;
    }

  /* if the local instance is the key server check sender's rx status */
  if (sender_live && is_local_key_server(mka))
    {
      if (((!xpn && llpn >= 0xc0000000) ||
           (xpn && llpn >= 0xc000000000000000ULL)) &&
          !mka->pn_exhaust_pending)
        {
          dot1x_debug_mka_peer(sender, "near PN exhaustion");
          mka->pn_exhaust_pending = 1;
        }
      if (lrx)
        receive_on(sender->peer);
      else
        receive_off(sender->peer);
    }

  /* if the sender is the key server check its tx status */
  if (is_principal(mka) && sender_live && is_remote_key_server(sender))
    set_server_transmitting(port, ltx);

  /* create or refresh peers from the live peer list of the PDU */
  for (i = 0; i < pdu->lpl.num; i++)
    {
      pdup = &pdu->lpl.tab[i];

      /* check if this is our own entry */
      if (!memcmp(&pdup->mi, &mka->cak->mka->mi, sizeof pdup->mi))
        continue;
      /* or the replaced MKA entry in case of MN exhaustion */
      if (mka->cak->replaced_mka)
        if (!memcmp(&pdup->mi, &mka->cak->replaced_mka->mi, sizeof pdu->mi))
          continue;

      if (!(mkap = dot1x_mka_peers_lookup(&mka->peers, &pdup->mi)) &&
          !(mkap = create_mkap(mka, &pdup->mi)))
        continue;

      if (pdup->mn < mkap->mn)
        continue;

      mkap->mn = pdup->mn;
      mkap->potential_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
    }

  /* handle KMD is the sender is the key server */
  if (mutually_live &&
      is_remote_key_server(sender) &&
      pdu->has_kmd)
    dot1x_kmdcpy(&mka->cak->kmd, &pdu->kmd);

  /* handle announcement */
  if (pdu->has_announcement)
    dot1x_process_announcement(intf, DOT1X_EAPOL_MKA, pdu->announcement);

  if (pdu->has_xpn)
    suspend_for =
      pdu->xpn_mst > DOT1X_MKA_SUSPENSION_LIMIT ?
      DOT1X_MKA_SUSPENSION_LIMIT : pdu->xpn_mst;
  else
    suspend_for = DOT1X_MKA_IN_SERVICE_UPGRADE_UNSUPPORTED;

  /* if the sender is live update sender's suspend_for value */
  if (sender_live && sender->suspend_for != suspend_for)
    {
      sender->suspend_for = suspend_for;

      if (suspend_for == DOT1X_MKA_IN_SERVICE_UPGRADE_UNSUPPORTED)
        dot1x_debug_mka_peer(sender, "in-service upgrade not supported");
      else
        dot1x_debug_mka_peer(sender, "updated suspendFor %u", suspend_for);
    }
}

static int
calculate_icv(
  Dot1xMka *mka, Dot1xIcv *icv,
  const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *msdu_buf, unsigned msdu_len)
{
  Dot1xIntf *intf = mka->cak->port->intf;
  Dot1xCryptoCmac *cmac = &intf->crypto_cmac;

  if (msdu_len < sizeof *icv)
    {
      dot1x_debug_discard(intf, "MSDU too short for ICV calculation");
      return 0;
    }

  dot1x_crypto_cmac_start(cmac, &mka->ick);
  dot1x_crypto_cmac_update(cmac, (void *)dst, sizeof *dst);
  dot1x_crypto_cmac_update(cmac, (void *)src, sizeof *src);
  dot1x_crypto_cmac_update(cmac, msdu_buf, msdu_len - sizeof *icv);
  dot1x_crypto_cmac_finish(cmac, (void *)icv->buf); /* XXX cast */

  return 1;
}

static Dot1xMkaPeer *
create_mkap(
  Dot1xMka *mka, const Dot1xMemberIdentifier *mi)
{
  Dot1xMkaPeer *mkap;
  Dot1xFormatBuffer fb;

  if (!(mkap = dot1x_mka_peers_insert(&mka->peers, mi)))
    {
      dot1x_error_mka(mka, "out of MKA peer entries");
      return NULL;
    }
  mkap->mka = mka;
  mka->peer_count++;
  trigger_mkpdu(mka);

  dot1x_debug_mka_peer(
    mkap, "created with MI %s",
    dot1x_format_member_identifier(&fb, &mkap->mi));
  return mkap;
}

static void
destroy_mkap(Dot1xMkaPeer *mkap)
{
  Dot1xMka *mka;

  if (!mkap)
    return;

  mka = mkap->mka;

  dot1x_debug_mka_peer(mkap, "being destroyed");

  make_not_live(mkap);
  macsec_off(mkap);

  assert(dot1x_mka_peers_remove(&mka->peers, &mkap->mi) == mkap);
  memset(mkap, 0, sizeof *mkap);
  mka->peer_count--;

  trigger_mkpdu(mka);
}

static int
make_live(Dot1xMkaPeer *mkap, const MacsecSci *sci, unsigned priority)
{
  Dot1xMka *mka = mkap->mka;
  Dot1xPort *port = mka->cak->port;
  Dot1xFormatBuffer fb;

  if (mkap->live)
    return 1;

  /* reference a peer entry */
  if (!(mkap->peer = refer_peer(port, sci, mka)))
    return 0;

  mkap->live = 1;
  mka->live_peer_count++;
  mka->live_peers_changed = 1;

  /* Stop distributing SAK. */
  port->distribute_sak = 0;

  add_remote_key_source(mkap, priority);

  dot1x_debug_mka_peer(mkap, "has SCI %s", dot1x_format_sci(&fb, sci));
  dot1x_debug_mka_peer(mkap, "live");

  trigger_mkpdu(mka);

  return 1;
}

static void
make_not_live(Dot1xMkaPeer *mkap)
{
  Dot1xMka *mka = mkap->mka;
  Dot1xPort *port = mka->cak->port;

  if (!mkap->live)
    return;

  dot1x_debug_mka_peer(mkap, "no longer live");

  if (is_local_key_server(mka))
    receive_off(mkap->peer);

  mkap->live = 0;
  mka->live_peer_count--;
  mka->live_peers_changed = 1;

  delete_remote_key_source(mkap);

  unref_peer(port, mkap->peer);
  mkap->peer = NULL;

  trigger_mkpdu(mka);
}

static void
macsec_on(Dot1xMkaPeer *mkap)
{
  Dot1xMka *mka = mkap->mka;

  if (mkap->macsec)
    return;

  mkap->macsec = 1;
  mka->macsec_peer_count++;

  if (mka->macsec_peer_count == 1)
    dot1x_debug_mka_peer(mkap, "is first peer requesting MACsec protection");
}

static void
macsec_off(Dot1xMkaPeer *mkap)
{
  Dot1xMka *mka = mkap->mka;

  if (!mkap->macsec)
    return;

  mkap->macsec = 0;
  mka->macsec_peer_count--;

  if (mka->macsec_peer_count == 0)
    dot1x_debug_mka_peer(mkap, "was last peer requesting MACsec protection");
}

static void
receive_on(Dot1xPeer *peer)
{
  Dot1xPort *port = peer->port;

  if (peer->receiving)
    return;

  peer->receiving = 1;
  port->receiving_peer_count++;
}

static void
receive_off(Dot1xPeer *peer)
{
  Dot1xPort *port = peer->port;

  if (!peer->receiving)
    return;

  peer->receiving = 0;
  port->receiving_peer_count--;
}

static void
clear_llpn(Dot1xPeer *peer)
{
  peer->llpn = 0;
  peer->llpn_changed = 0;
}

static void
transmit_sa_in_use_on(Dot1xPort *port)
{
  if (port->transmit_sa_in_use)
    return;

  port->transmit_sa_in_use = 1;
  dot1x_debug_mka_secy(
    port, "TransmitSA.inUse = %s", dot1x_bool(port->transmit_sa_in_use));

  /* set AN used by PN number update */
  port->transmit_sa_an = port->new_transmit_sa_an;
}

static void
transmit_sa_in_use_off(Dot1xPort *port)
{
  if (!port->transmit_sa_in_use)
    return;

  port->transmit_sa_in_use = 0;
  dot1x_debug_mka_secy(
    port, "TransmitSA.inUse = %s", dot1x_bool(port->transmit_sa_in_use));
}

static void
receive_sa_in_use_on(Dot1xPeer *peer)
{
  Dot1xPort *port = peer->port;

  if (peer->receive_sa_in_use)
    return;

  peer->receive_sa_in_use = 1;
  dot1x_debug_mka_secy_peer(
    peer, "ReceiveSA.inUse = %s", dot1x_bool(peer->receive_sa_in_use));

  port->using_peer_count++;
}

static void
receive_sa_in_use_off(Dot1xPeer *peer)
{
  Dot1xPort *port = peer->port;

  if (!peer->receive_sa_in_use)
    return;

  peer->receive_sa_in_use = 0;
  dot1x_debug_mka_secy_peer(
    peer, "ReceiveSA.inUse = %s", dot1x_bool(peer->receive_sa_in_use));

  port->using_peer_count--;
}

static void
add_local_key_source(Dot1xMka *mka, unsigned priority)
{
  Dot1xCak *cak = mka->cak;
  Dot1xPort *port = cak->port;
  unsigned supplicant = cak->supplicant;

  add_key_source(
    port, &mka->key_source, mka, NULL,
    cak->pairwise, supplicant, priority,
    &port->intf->local_address, port->portid);
}

static void
delete_local_key_source(Dot1xMka *mka)
{
  delete_key_source(&mka->key_source);
}

static void
add_remote_key_source(Dot1xMkaPeer *mkap, unsigned priority)
{
  Dot1xPeer *peer = mkap->peer;
  Dot1xMka *mka = mkap->mka;
  Dot1xCak *cak = mka->cak;
  Dot1xPort *port = cak->port;
  unsigned supplicant = cak->authenticator;

  add_key_source(
    port, &mkap->key_source, mka, mkap,
    cak->pairwise, supplicant, priority,
    &peer->peer_address, peer->peer_portid);

  change_mka_key_server(mka);
}

static void
delete_remote_key_source(Dot1xMkaPeer *mkap)
{
  delete_key_source(&mkap->key_source);
}

static void
update_local_key_source(Dot1xMka *mka)
{
  update_key_source(
    &mka->key_source,
    mka->cak->port->intf->ctrl.actor_priority,
    mka->live_peer_count);
}

static void
update_remote_key_source(Dot1xMkaPeer *mkap, unsigned priority,
    unsigned live_peer_count)
{
  update_key_source(&mkap->key_source, priority, live_peer_count);
}

static void
change_mka_key_server(Dot1xMka *mka)
{
  Dot1xMkaKeySource *key_source = &mka->key_source;
  Dot1xMkaPeer *mka_peer = NULL;
  Dot1xCak *cak = mka->cak;
  Dot1xIntf *intf = cak->port->intf;

  update_local_key_source(mka);

  if (key_source->priority == 255 ||
      (cak->supplicant && cak->type == DOT1X_CAK_EAP))
    key_source = NULL;

  if (!cak->authenticator || cak->type != DOT1X_CAK_EAP)
    {
      while ((mka_peer = dot1x_mka_peers_next(&mka->peers, mka_peer)))
        {
          Dot1xMkaKeySource *peer_source = &mka_peer->key_source;

          if (mka_peer->live && peer_source->priority < 255 &&
              peer_source->willing_key_server)
            {
              if (key_source == NULL)
                key_source = peer_source;
              else
              if (peer_source->priority < key_source->priority)
                key_source = peer_source;
              else
              if (peer_source->priority == key_source->priority &&
                  memcmp(peer_source->sci, key_source->sci, 6) < 0)
                key_source = peer_source;
            }
        }
    }

  if (!key_source && mka->key_server)
    {
      dot1x_debug_key_server_mka(mka, "key server lost");
      intf->mka_changed = 1;
    }
  else
  if (key_source && key_source != mka->key_server)
    {
      if (key_source == &key_source->mka->key_source)
        dot1x_debug_key_server_mka(key_source->mka, "became key server");
      else
        dot1x_debug_key_server_mka_peer(
                key_source->mka_peer, "became key server");
      intf->mka_changed = 1;
    }

  mka->key_server = key_source;
  change_principal(mka->cak->port);
}

static void
add_key_source(
  Dot1xPort *port, Dot1xMkaKeySource *key_source,
  Dot1xMka *mka, Dot1xMkaPeer *mkap,
  unsigned pairwise, unsigned supplicant, unsigned priority,
  const MacsecAddress *address, unsigned portid)
{
  key_source->mka = mka;
  key_source->mka_peer = mkap;
  key_source->priority = priority;
  memcpy(key_source->sci, address->buf, 6);
  key_source->sci[6] = (portid >> 8) & 0xff;
  key_source->sci[7] = portid & 0xff;
  key_source->pairwise = pairwise;
  key_source->supplicant = supplicant;

  insert_key_source(port, key_source);
}

static void
delete_key_source(Dot1xMkaKeySource *key_source)
{
  remove_key_source(key_source);
  memset(key_source, 0, sizeof *key_source);
}

static void
update_key_source(Dot1xMkaKeySource *key_source, unsigned priority,
  unsigned live_peer_count)
{
  Dot1xMka *mka = key_source->mka;
  Dot1xPort *port;

  port = mka->cak->port;

  /* Check if anything has changed */
  if ((priority == key_source->priority) &&
      (live_peer_count == key_source->live_peer_count))
    return;

  remove_key_source(key_source);
  key_source->priority = priority;
  key_source->live_peer_count = live_peer_count;
  insert_key_source(port, key_source);
}

void
insert_key_source(Dot1xPort *port, Dot1xMkaKeySource *source)
{
  int diff_supplicant, diff_priority, diff_sci, diff_pairwise,
    diff_live_peer_count;
  Dot1xMkaKeySource *other;
  Dot1xMkaKeySource *compared;
  Dot1xCak* parent_cak;
  /* look up the first worse or equal key server candidate and if
     found then insert before that, otherwise append to end of list */

  other = NULL;
  while ((other = macsec_list_next(
            &port->key_sources, offsetof(Dot1xMkaKeySource, link), other)))
    {
      compared = other;
      /* for local key sources consider also the parent key source */
      if (source->mka->cak->preshared)
        {
          parent_cak = dot1x_caks_lookup(&port->intf->caks,
            &other->mka->cak->parent_ckn);
          if (parent_cak && !other->mka_peer && parent_cak->mka)
            compared = &parent_cak->mka->key_source;
        }

      diff_supplicant = compared->supplicant - source->supplicant;
      diff_priority = compared->priority - source->priority;
      diff_sci = memcmp(compared->sci, source->sci, sizeof compared->sci);
      diff_pairwise = compared->pairwise - source->pairwise;
      /* The key source with most live peers gets higher priority in the
        case of PSK CAKs. */
      if (source->mka->cak->preshared && compared->mka->cak->preshared)
        diff_live_peer_count = compared->live_peer_count - source->live_peer_count;
      else
        diff_live_peer_count = 0;

      if ((diff_supplicant > 0 ||
           (diff_supplicant == 0 &&
            (diff_priority > 0 ||
             (diff_priority == 0 &&
              (diff_sci > 0 ||
               (diff_sci == 0 &&
                (diff_pairwise > 0 ||
                 (diff_pairwise == 0 &&
                  diff_live_peer_count <= 0)))))))))
          break;
    }

  if (other)
    macsec_list_insbefore(&other->link, &source->link);
  else
    macsec_list_append(&port->key_sources, &source->link);
}

void
remove_key_source(Dot1xMkaKeySource *source)
{
  macsec_list_remove(&source->link);
}

void
set_principal(Dot1xPort *port, Dot1xMka *principal)
{
  if (principal != port->principal)
    {
      if (principal)
        dot1x_debug_mka_actor(principal, "selected principal.");
      else
        dot1x_debug_port(port, "no principal selected");

      port->principal = principal;

      port->intf->mka_changed = 1;
    }

  /* update electedSelf */
  if (principal && principal->key_server == &principal->key_source)
    set_elected_self(port, 1);
  else
    set_elected_self(port, 0);
}

static int
is_local_key_server(Dot1xMka *mka)
{
  return is_mka_key_server(mka) && is_principal(mka);
}

static int
is_mka_key_server(Dot1xMka *mka)
{
  return mka->key_server == &mka->key_source;
}

static int
is_remote_key_server(Dot1xMkaPeer *mkap)
{
  return mkap->mka->key_server == &mkap->key_source;
}

static int
is_principal(Dot1xMka *mka)
{
  return mka == get_principal(mka->cak->port);
}

static int
key_server_is_local(Dot1xPort *port)
{
  Dot1xMkaKeySource *key_server;

  return
    (key_server = dot1x_get_key_server(port)) &&
    !key_server->mka_peer;
}

static Dot1xMka *
get_principal(Dot1xPort *port)
{
  return port->principal;
}

static void
change_principal(
  Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;
  Dot1xMka *mka = NULL;
  Dot1xMkaKeySource *key_source = NULL;
  Dot1xMka *principal = port->principal;

  int found_principal = 0;

  while ((mka = dot1x_mkas_next(&intf->mkas, mka)))
    {
      Dot1xMkaKeySource *mka_source = mka->key_server;

      if (mka->cak->port != port)
        continue;

      if (mka == principal)
        found_principal = 1;

      if (mka_source == NULL || mka_source->priority == 255)
        continue;

      if (key_source == NULL)
        key_source = mka_source;
      else
      if (mka_source->priority < key_source->priority)
        key_source = mka_source;
      else
      if (mka_source->priority == key_source->priority &&
          memcmp(mka_source->sci, key_source->sci, 6) < 0)
        key_source = mka_source;
    }

  while ((mka = dot1x_mkas_next(&intf->mkas, mka)))
    {
      Dot1xMkaKeySource *mka_source = mka->key_server;

      if (mka->cak->port != port)
        continue;

      if (key_source &&
          mka_source &&
          (!port->distributed_ki.kn || mka_source->live_peer_count > 0) &&
          mka_source->priority == key_source->priority &&
          memcmp(mka_source->sci, key_source->sci, 6) == 0)
        mka->potential_principal = 1;
      else
        mka->potential_principal = 0;
    }

  if (key_source == NULL)
  {
    principal = NULL;
  }
  else
  if (key_source == &key_source->mka->key_source || !port->distributed_ki.kn)
    {
      Dot1xMkaKeySource *source = NULL;

      principal = NULL;
      while ((source = macsec_list_next(
                &port->key_sources,
                offsetof(Dot1xMkaKeySource, link),
                source)))
        {
          if ((source == &source->mka->key_source ||
               !port->distributed_ki.kn) &&
              source->mka->potential_principal)
            {
              if (principal == NULL ||
                  principal->key_server->live_peer_count <
                          source->live_peer_count)
                principal = source->mka;
            }
        }
    }

  if (!principal && is_suspended(port) && found_principal)
      principal = port->principal;

  if (!principal && is_suspended(port))
    {
      Dot1xMkaKeySource *source =
        macsec_list_next(
                &port->key_sources, offsetof(Dot1xMkaKeySource, link), NULL);

      if (source)
        principal = source->mka;
    }

  /* clear potential old SAK if a new one is not expected */
  if (port->principal && !principal && !is_suspended(port))
  {
    dot1x_debug_sak(port, "clearing SAK because of no key server");
    change_sak(
            port, intf->ctrl.cipher_suite, intf->ctrl.confidentiality_offset,
            NULL, 0, NULL, 0);
  }

  set_principal(port, principal);
}

static void
change_sak(
  Dot1xPort *port, MacsecCipherSuite cs, MacsecConfidentialityOffset co,
  const Dot1xMemberIdentifier *mi, uint32_t kn,
  const MacsecKey *sak, unsigned an)
{
  Dot1xMka *mka;
  Dot1xSakPn *sak_pn;
  Dot1xFormatBuffer fb;
  uint64_t next_pn = 1;

  set_cipher_suite(port, cs);
  set_cipher_offset(port, co);

  mka = get_principal(port);

  /* reset peer receive statuses */
  foreach_peer(port, receive_off);

  /* if new SAK exists get stored KI-nextPN info, if any, for the key server */
  if (mka && kn && (sak_pn = dot1x_sak_pns_remove(&mka->sak_pns, mi)))
    {
      /* if key number matches use stored PN */
      if (sak_pn->kn == kn)
        next_pn = sak_pn->next_pn;
    }

  /* save KI-nextPN info and OLPN of the old SAK if there was one */
  if (mka && port->distributed_ki.kn)
    {
      if (!(sak_pn =
            dot1x_sak_pns_lookup(&mka->sak_pns, &port->distributed_ki.mi)) &&
          !(sak_pn =
            dot1x_sak_pns_insert(&mka->sak_pns, &port->distributed_ki.mi)))
        {
          dot1x_error_mka(mka, "out of KI-nextPN entries");
          mka->replace = 1;
          dot1x_debug_mka_cp(
            port, "distributedKI = %s",
            dot1x_format_key_identifier(&fb, &port->distributed_ki));
          return;
        }
      sak_pn->kn = port->distributed_ki.kn;
      sak_pn->next_pn = pn_history_last(
        &port->transmit_sa_next_pn_history[port->transmit_sa_an]);
    }

  /* set up new SAK if there is one, otherwise clear SAK */
  if (kn)
    {
      /* store given SAK into output variables */
      memcpy(&port->distributed_sak, sak, sizeof port->distributed_sak);
      memcpy(&port->distributed_ki.mi, mi, sizeof port->distributed_ki.mi);
      port->distributed_ki.kn = kn;
      dot1x_debug_mka_cp(
        port, "distributedKI = %s",
        dot1x_format_key_identifier(&fb, &port->distributed_ki));

      /* update current AN */
      port->distributed_an = an;
      dot1x_debug_mka_cp(port, "distributedAN = %u", port->distributed_an);

      /* set up the AN to use after the next SAK change */
      port->next_an = an + 1;
      port->next_an &= 3;

      /* reset receive LLPN information */
      foreach_peer(port, clear_llpn);

      /* jump-start transmit PN history for correct LLPN reporting */
      pn_history_fill(&port->transmit_sa_next_pn_history[an], next_pn);

      /* set PN for SA create */
      port->new_transmit_sa_next_pn = next_pn;

      /* signal CP */
      set_new_sak(port, 1);
    }
  else if (port->distributed_ki.kn)
    {
      memset(&port->distributed_sak, 0, sizeof port->distributed_sak);
      memset(&port->distributed_ki, 0, sizeof port->distributed_ki);
      dot1x_debug_mka_cp(
        port, "distributedKI = %s",
        dot1x_format_key_identifier(&fb, &port->distributed_ki));
      set_new_sak(port, 0);
    }

  /* ensure a previously installed key is not any more to enable SAs */
  port->receive_sas_enabled = 0;
  memset(&port->macsec_ki, 0, sizeof port->macsec_ki);
}

static Dot1xCak *
change_cak(Dot1xCak *cak, const Dot1xCkn *ckn, const MacsecKey *key)
{
  Dot1xPort *port = cak->port;
  Dot1xCakType type;
  Dot1xNid *nid;

  if ((!ckn && !port->distributed_cak) ||
      (ckn && port->distributed_cak &&
       !dot1x_ckncmp(ckn, &port->distributed_cak->ckn)))
    return NULL;

  if (ckn && key)
    {
      type = DOT1X_CAK_DISTRIBUTED;
      nid = &cak->network->nid;

      dot1x_remove_disabled_caks(port->intf, type, nid);

      /* the following may fail and leave distributed_cak NULL */
      port->distributed_cak =
        dot1x_insert_cak(port->intf, ckn, key, type, nid);
      if (port->distributed_cak)
        {
          dot1x_inherit_cak(port->distributed_cak, cak);
          /* set the distributed - parent relationship */
          if (key_server_is_local(port) && cak->preshared)
            {
              /* only key server knows the parent CAK */
              dot1x_ckncpy(&port->distributed_cak->parent_ckn, &cak->ckn);
            }

          if (cak->intf->cak_cache_callback)
            cak->intf->cak_cache_callback(
              DOT1X_CAK_CACHE_INSERT, port->distributed_cak);
        }
    }
  else
    {
      if (port->distributed_cak)
        {
          port->distributed_cak->active_ticks = 1;
          port->distributed_cak = NULL;
        }
    }

  /* jump SAK generation throttle timer after CAK change */
  port->sak_hold_ticks = 0;

  port->intf->mka_changed = 1;

  return port->distributed_cak;
}

static Dot1xPeer *
refer_peer(Dot1xPort *port, const MacsecSci *sci, Dot1xMka *mka)
{
  Dot1xIntf *intf = port->intf;
  Dot1xPeer *peer;
  Dot1xFormatBuffer fb;

  if (port != &intf->rport)
    {
      /* for a virtual port, refer to its containing peer object */
      peer = (void *)((unsigned char *)port - offsetof(Dot1xPeer, vport));

      /* verify address discovered earlier from EAP */
      if (memcmp(&sci->address, &peer->peer_address, sizeof sci->address))
        {
          dot1x_error_peer(peer, "MAC address mismatch");
          return NULL;
        }

      /* count a reference to the peer object */
      dot1x_reref_peer(intf, peer);
    }
  else
    {
      /* for the real port, get peer object by MAC address */
      if (!(peer = dot1x_refer_peer(intf, DOT1X_PEER_REAL, &sci->address)))
        return NULL;
    }

  /* verify remote port identifier if it has been already set */
  if (peer->mka_reference_count > 0 && sci->portid != peer->peer_portid)
    {
      dot1x_error_peer(peer, "port identifier mismatch");
      dot1x_unref_peer(intf, peer);
      return NULL;
    }

  if (is_suspended(port) && peer->mka_reference_count <= 0)
    {
      if (!peer->macsec_peer)
        {
          mka->suspension_unseen_peer = 1;
        }
    }

  /* first live MKA peer reveals the remote port identifier and allows
     creating the MACsec peer object */
  if (peer->mka_reference_count <= 0 && !peer->macsec_peer)
    {
      peer->peer_portid = sci->portid;

      dot1x_debug_mka_secy(
        port, "createReceiveSC(%s/%u)",
        dot1x_format_address(&fb, &peer->peer_address), peer->peer_portid);

      if (!(peer->macsec_peer =
            macsec_create_peer(
              port->intf->macsec_intf, port->portid,
              &peer->peer_address, peer->peer_portid)))
        {
          dot1x_error_port(port, "cannot create receive SC");
          dot1x_unref_peer(intf, peer);
          return NULL;
        }

      if (port != &intf->rport)
        {
          dot1x_debug_port(port, "virtual port created");

          dot1x_debug_port(
            port, "is associated with peer station %s",
            dot1x_format_address(&fb, &peer->peer_address));
        }

      /* keep track of per-port highest live peer station count */
      port->peer_count++;
      if (port->peer_count > port->highest_peer_count)
        port->highest_peer_count = port->peer_count;

      /* if there s an installed SAK (createSAs() already done)
         create SA now */
      if (memcmp(
                  &port->macsec_ki,
                  &MACSEC_KI_INVALID,
                  sizeof port->macsec_ki) != 0)
        {
          create_receive_sa(peer);
          /* and enable it if enableReceiveSAs() has been done already */
          if (port->receive_sas_enabled)
            enable_receive_sa(peer);
        }
    }

  peer->mka_reference_count++;
  return peer;
}

static void
unref_peer(Dot1xPort *port, Dot1xPeer *peer)
{
  Dot1xIntf *intf = port->intf;

  if (--peer->mka_reference_count <= 0)
    {
      if (is_suspended(port))
        {
          dot1x_debug_mka_secy_peer(peer, "suspend");
          return;
        }

      receive_sa_in_use_off(peer);

      if (port->distributed_cak_source == peer)
        port->distributed_cak_source = NULL;

      if (peer->macsec_peer)
        {
          macsec_destroy_peer(peer->macsec_peer);

          peer->macsec_peer = 0;

          if (port == &peer->vport)
            dot1x_debug_port(port, "virtual port destroyed");

          /* keep track of per-port highest live peer station count */
          port->peer_count--;
          if (port->peer_count <= 0)
            port->highest_peer_count = 0;
        }
    }

  dot1x_unref_peer(intf, peer);
}

static void
create_transmit_sa(Dot1xPort *port)
{
  unsigned an = port->distributed_an;
  uint64_t next_pn = port->new_transmit_sa_next_pn;

  /* remember AN being created */
  port->new_transmit_sa_an = an;

  dot1x_debug_mka_secy(
    port, "createTransmitSA(%u, %llu, %p)",
    an, (unsigned long long)next_pn, &port->macsec_ki);

  macsec_create_transmit_sa(
    port->intf->macsec_intf, port->portid, an, next_pn,
    &port->macsec_ki, &port->intf->ssci);

  port->has_sa = 1;
}

static void
create_receive_sa_checked(Dot1xPeer *peer)
{
  /* skip peer that has no MACsec-level peer */
  if (peer->mka_reference_count <= 0)
    return;

  create_receive_sa(peer);
}

static void
create_receive_sa(Dot1xPeer *peer)
{
  Dot1xPort *port = peer->port;
  unsigned an;
  uint64_t lowest_pn;

  an = port->distributed_an;
  lowest_pn = peer->llpn;
  if (lowest_pn < 1)
    lowest_pn = 1;

  /* remember AN being created */
  peer->new_receive_sa_an = an;

  dot1x_debug_mka_secy(
    port, "createReceiveSA(%u, %llu, %p)", an, (unsigned long long)lowest_pn,
    &port->macsec_ki);

  macsec_create_receive_sa(
    peer->macsec_peer, an, lowest_pn, &port->macsec_ki, &peer->ssci);

  peer->has_sa = 1;
}

static void
enable_receive_sa_checked(Dot1xPeer *peer)
{
  /* skip peer that has no MACsec-level peer */
  if (peer->mka_reference_count <= 0)
    return;

  enable_receive_sa(peer);
}

static void
enable_receive_sa(Dot1xPeer *peer)
{
  receive_sa_in_use_off(peer);

  dot1x_debug_mka_secy_peer(peer, "ReceiveSA.enableReceive");

  macsec_enable_receive(peer->macsec_peer, peer->new_receive_sa_an);

  /* assume immediate inUse = TRUE */
  receive_sa_in_use_on(peer);
}

static void
set_enabled(Dot1xIntf *intf, int value)
{
  if (intf->mka_enabled == value)
    return;

  intf->mka_enabled = value;
  dot1x_debug_logon_mka(
    &intf->rport, "mka.enabled = %s", dot1x_bool(intf->mka_enabled));
  intf->mka_changed = 1;
}

static void
set_all_receiving(Dot1xPort *port, int value)
{
  if (port->all_receiving == value)
    return;

  port->all_receiving = value;
  dot1x_debug_mka_cp(
    port, "allReceiving = %s", dot1x_bool(port->all_receiving));
  port->intf->mka_changed = 1;
}

static void
set_server_transmitting(Dot1xPort *port, int value)
{
  if (port->server_transmitting == value)
    return;

  port->server_transmitting = value;
  dot1x_debug_mka_cp(
    port, "serverTransmitting = %s", dot1x_bool(port->server_transmitting));
  port->intf->mka_changed = 1;
}

static void
set_using_receive_sas(Dot1xPort *port, int value)
{
  if (port->using_receive_sas == value)
    return;

  port->using_receive_sas = value;
  dot1x_debug_mka_cp(
    port, "usingReceiveSAs = %s", dot1x_bool(port->using_receive_sas));
  port->intf->mka_changed = 1;
}

static void
set_using_transmit_sa(Dot1xPort *port, int value)
{
  if (port->using_transmit_sa == value)
    return;

  port->using_transmit_sa = value;
  dot1x_debug_mka_cp(
    port, "usingTransmitSA = %s", dot1x_bool(port->using_transmit_sa));
  port->intf->mka_changed = 1;
}

static void
set_elected_self(Dot1xPort *port, int value)
{
  if (port->elected_self == value)
    return;

  port->elected_self = value;
  dot1x_debug_mka_cp(
    port, "electedSelf = %s", dot1x_bool(port->elected_self));
  port->intf->mka_changed = 1;
}

static void
set_new_sak(Dot1xPort *port, int value)
{
  if (port->new_sak == value)
    return;

  port->new_sak = value;
  dot1x_debug_mka_cp(port, "newSAK = %s", dot1x_bool(port->new_sak));
  port->intf->mka_changed = 1;
}

static void
set_chgd_server(Dot1xPort *port, int value)
{
  if (port->chgd_server == value)
    return;

  port->chgd_server = value;
  dot1x_debug_mka_cp(port, "chgdServer = %s", dot1x_bool(port->chgd_server));
  port->intf->mka_changed = 1;
}

static void
set_cipher_suite(Dot1xPort *port, MacsecCipherSuite value)
{
  if (port->cipher_suite == value)
    return;

  port->cipher_suite = value;
  dot1x_debug_mka_cp(
    port, "cipherSuite = %s", dot1x_cipher_suite(port->cipher_suite));
  port->intf->mka_changed = 1;
}

static void
set_cipher_offset(Dot1xPort *port, MacsecConfidentialityOffset value)
{
  if (port->cipher_offset == value)
    return;

  port->cipher_offset = value;
  dot1x_debug_mka_cp(
    port, "cipherOffset = %s",
    dot1x_confidentiality_offset(port->cipher_offset));
  port->intf->mka_changed = 1;
}

static void
update_intf_suspended_while(
        Dot1xIntf *intf)
{
    Dot1xPeer *peer = NULL;
    unsigned suspended_while = 0xffffffff;

    while ((peer = dot1x_peers_next(&intf->peers, peer)))
      {
        if (peer->port->can_suspend &&
            peer->port->suspended_while != 0 &&
            peer->port->suspended_while < suspended_while)
          suspended_while = peer->port->suspended_while;
      }

    if (suspended_while == 0xffffffff)
      suspended_while = 0;

    if (intf->stat.suspended_while != suspended_while)
    {
      intf->stat.suspended_while = suspended_while;

      dot1x_debug_control(intf, "suspendedWhile = %d", suspended_while);
    }
}

static void
set_can_suspend(Dot1xPort *port, unsigned value)
{
  if (port->can_suspend == value)
    return;

  port->can_suspend = value;

  dot1x_debug_mka_cp(port, "canSuspend = %u", port->can_suspend);
  port->intf->mka_changed = 1;

  update_intf_suspended_while(port->intf);
}


static const Dot1xMkpduPeer *
find_pdup(
  const Dot1xMkpduPeerList *pl, const Dot1xMemberIdentifier *mi)
{
  const Dot1xMkpduPeer *p;
  int i;

  for (i = 0; i < pl->num; i++)
    {
      p = &pl->tab[i];
      if (!memcmp(&p->mi, mi, sizeof p->mi))
        return p;
    }

  return NULL;
}

static int
test_kek_ick_derivation(void)
{
  const unsigned char test_key[] = {
    0x13, 0x5b, 0xd7, 0x58, 0xb0, 0xee, 0x5c, 0x11,
    0xc5, 0x5f, 0xf6, 0xab, 0x19, 0xfd, 0xb1, 0x99};
  const unsigned char test_label_kek[] = {
    0x49, 0x45, 0x45, 0x45, 0x38, 0x30, 0x32, 0x31,
    0x20, 0x4b, 0x45, 0x4b};
  const unsigned char test_label_ick[] = {
    0x49, 0x45, 0x45, 0x45, 0x38, 0x30, 0x32, 0x31,
    0x20, 0x49, 0x43, 0x4b};
  const unsigned char test_context[] = {
    0x96, 0x43, 0x7a, 0x93, 0xcc, 0xf1, 0x0d, 0x9d,
    0xfe, 0x34, 0x78, 0x46, 0xcc, 0xe5, 0x2c, 0x7d};
  const unsigned char test_length[] = {
    0x00, 0x80};
  const unsigned char test_output_kek[] = {
    0x8f, 0x5a, 0x38, 0x4c, 0x15, 0xd6, 0xae, 0x93,
    0x02, 0xb4, 0x62, 0xe3, 0x63, 0xd0, 0x3c, 0xa6};
  const unsigned char test_output_ick[] = {
    0x8f, 0x1c, 0x5c, 0xb1, 0xc8, 0xed, 0x2e, 0x5f,
    0x04, 0x79, 0x06, 0xe0, 0x47, 0x3a, 0xad, 0x4d};

  Dot1xIntf intf;
  Dot1xMka mka;
  Dot1xCak cak;
  MacsecKey key;
  MacsecKey kek;
  MacsecKey ick;
  int ok = 1;

  mka.cak = &cak;

  memcpy(key.buf, test_key, sizeof test_key);
  key.len = sizeof test_key;

  assert(strlen(kek_label) == sizeof test_label_kek);
  assert(!memcmp(kek_label, test_label_kek, sizeof test_label_kek));

  assert(strlen(ick_label) == sizeof test_label_ick);
  assert(!memcmp(ick_label, test_label_ick, sizeof test_label_ick));

  memcpy(cak.ckn.buf, test_context, sizeof test_context);
  cak.ckn.len = sizeof test_context;

  kek.len = 16;
  assert(((test_length[0] << 8) | test_length[1]) == (kek.len << 3));

  ick.len = 16;
  assert(((test_length[0] << 8) | test_length[1]) == (ick.len << 3));

  dot1x_derive_key(
    &intf, kek.buf, kek.len, &key, kek_label,
    context_kek_ick, &mka);

  dot1x_derive_key(
    &intf, ick.buf, ick.len, &key, ick_label,
    context_kek_ick, &mka);

  assert(kek.len == sizeof test_output_kek);
  assert(ick.len == sizeof test_output_ick);

  ok &= !memcmp(kek.buf, test_output_kek, kek.len);
  ok &= !memcmp(ick.buf, test_output_ick, ick.len);

  return ok;
}

static int
test_sak_derivation(void)
{
  const unsigned char test_key[] = {
    0x13, 0x5b, 0xd7, 0x58, 0xb0, 0xee, 0x5c, 0x11,
    0xc5, 0x5f, 0xf6, 0xab, 0x19, 0xfd, 0xb1, 0x99};
  const unsigned char test_label[] = {
    0x49, 0x45, 0x45, 0x45, 0x38, 0x30, 0x32, 0x31,
    0x20, 0x53, 0x41, 0x4b};
  const unsigned char test_ks_nonce[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
  const unsigned char test_mi_value_list[] = {
    0xcd, 0x42, 0x1c, 0xf8, 0x6b, 0xa4, 0x57, 0x93,
    0x86, 0x57, 0x67, 0x5b, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x0d, 0x1f, 0x36, 0xcf};
  const unsigned char test_kn[] = {
    0x00, 0x00, 0x00, 0x01};
  const unsigned char test_length[] = {
    0x00, 0x80};
  const unsigned char test_output[] = {
    0x04, 0x52, 0x05, 0x92, 0x58, 0x31, 0xae, 0x59,
    0xc1, 0x45, 0x50, 0xed, 0x59, 0xcc, 0x00, 0x3d};

  Dot1xIntf intf;
  Dot1xMka mka;
  Dot1xMkaPeer *mkap1, *mkap2;
  Dot1xMemberIdentifier mi;
  SakDerivationArg sda;
  MacsecKey key;
  MacsecKey sak;

  memcpy(key.buf, test_key, sizeof test_key);
  key.len = sizeof test_key;

  assert(strlen(sak_label) == sizeof test_label);
  assert(!memcmp(sak_label, test_label, sizeof test_label));

  memcpy(sda.nonce.buf, test_ks_nonce, sizeof test_ks_nonce);
  sda.nonce.len = sizeof test_ks_nonce;

  assert(2 * sizeof mi.buf == sizeof test_mi_value_list);
  dot1x_mka_peers_init(&mka.peers);
  /* insert peers in order mkap1, mkap2 in the dictionary */
  mi.buf[0] = 0;
  mkap1 = dot1x_mka_peers_insert(&mka.peers, &mi);
  mi.buf[0] = 1;
  mkap2 = dot1x_mka_peers_insert(&mka.peers, &mi);
  /* patch peer MIs with test values */
  memcpy(mkap1->mi.buf, test_mi_value_list, sizeof mkap1->mi.buf);
  memcpy(
    mkap2->mi.buf, test_mi_value_list + sizeof mkap2->mi.buf,
    sizeof mkap2->mi.buf);

  mka.kn =
    (test_kn[0] << 24) | (test_kn[1] << 16) | (test_kn[2] << 8) | test_kn[3];

  sak.len = 16;
  assert(((test_length[0] << 8) | test_length[1]) == (sak.len << 3));

  sda.mka = &mka;

  dot1x_derive_key(
    &intf, sak.buf, sak.len, &key, sak_label,
    context_sak, &sda);

  assert(sak.len == sizeof test_output);
  if (memcmp(sak.buf, test_output, sak.len))
    return 0;

  return 1;
}

static void
context_kek_ick(void *arg, Dot1xCryptoCmac *cmac)
{
  Dot1xMka *mka = arg;
  const Dot1xCkn *ckn;
  unsigned len, pad;

  ckn = &mka->cak->ckn;

  if (ckn->len < 16)
    {
      len = ckn->len;
      pad = 16 - ckn->len;
    }
  else
    {
      len = 16;
      pad = 0;
    }
  dot1x_crypto_cmac_update(cmac, ckn->buf, len);
  dot1x_crypto_cmac_update(cmac, zero, pad);
}

static void
context_sak(void *arg, Dot1xCryptoCmac *cmac)
{
  SakDerivationArg *sda = arg;
  Dot1xMka *mka = sda->mka;
  Dot1xMkaPeer *mkap;
  unsigned char kn[4];

  /* KS-nonce */
  dot1x_crypto_cmac_update(cmac, sda->nonce.buf, sda->nonce.len);

  /* MI-value list */
  mkap = NULL;
  while ((mkap = dot1x_mka_peers_next(&mka->peers, mkap)))
    dot1x_crypto_cmac_update(cmac, mkap->mi.buf, sizeof mkap->mi.buf);

  /* KN */
  kn[0] = (mka->kn >> 24) & 0xff;
  kn[1] = (mka->kn >> 16) & 0xff;
  kn[2] = (mka->kn >> 8) & 0xff;
  kn[3] = mka->kn & 0xff;
  dot1x_crypto_cmac_update(cmac, kn, sizeof kn);
}

static void
pn_history_fill(Dot1xPnHistory *pnh, uint64_t pn)
{
  int i;
  for (i = 0; i < sizeof pnh->ring / sizeof pnh->ring[0]; i++)
    pnh->ring[i] = pn;
}

static void
pn_history_update(Dot1xPnHistory *pnh, uint64_t pn)
{
  pnh->last++;

  if (pnh->last >= sizeof pnh->ring  / sizeof pnh->ring[0])
    pnh->last = 0;

  pnh->ring[pnh->last] = pn;
}

static uint64_t
pn_history_last(Dot1xPnHistory *pnh)
{
  return pnh->ring[pnh->last];
}

static unsigned
mka_greatest_suspend_for(Dot1xMka *mka)
{
  unsigned suspend_for = mka->cak->port->suspend_for;
  Dot1xMkaPeer *mkap = NULL;

  while ((mkap = dot1x_mka_peers_next(&mka->peers, mkap)))
    {
      if (!mkap->live)
        continue;

      if (mkap->suspend_for == DOT1X_MKA_IN_SERVICE_UPGRADE_UNSUPPORTED)
        continue;

      if (suspend_for < mkap->suspend_for)
        suspend_for = mkap->suspend_for;
    }

  return suspend_for;
}

static unsigned
mka_has_missing_peer(Dot1xMka *mka)
{
  Dot1xPort *port = mka->cak->port;
  Dot1xIntf *intf = port->intf;
  Dot1xPeer *peer = NULL;
  unsigned has = 0;

  while ((peer = dot1x_peers_next(&intf->peers, peer)))
    {
      Dot1xMkaPeer *mkap = NULL;
      if (peer->port != port)
        continue;

      if (peer->mka_reference_count == 0)
        {
          has = 1;
          break;
        }

      while ((mkap = dot1x_mka_peers_next(&mka->peers, mkap)))
        {
          if (!mkap->live)
            continue;

          if (mkap->peer == peer)
            break;
        }

      if (mkap == NULL)
        {
          has = 1;
          break;
        }
    }

  return has;
}

static void
step_suspension_keyserver(Dot1xMka *mka)
{
  Dot1xPort *port = mka->cak->port;

  unsigned has_missing_peer = mka_has_missing_peer(mka);
  unsigned greatest_suspend_for = mka_greatest_suspend_for(mka);
  unsigned suspended_while = dot1x_to_secs(port->suspended_while_ticks);
  unsigned local_suspended_while =
    dot1x_to_secs(port->local_suspended_while_ticks);

  if (is_suspended(port))
    {
      set_can_suspend(
              port,
              (!mka->keyserver_suspend_ticks && port->suspend_for > 0));

      if (mka->suspension_unseen_peer)
        {
          dot1x_suspension_stop(port, "unseen peer");
        }
      else
      if (greatest_suspend_for == DOT1X_MKA_IN_SERVICE_UPGRADE_UNSUPPORTED)
        {
          dot1x_suspension_stop(port, "not supporting peer");
        }
      else
      if (!has_missing_peer && greatest_suspend_for == 0 &&
          (!port->resuming_suspend || !port->local_suspended_while_ticks))
        {
          dot1x_suspension_stop(port, "all resumed");
        }
      else
        {
          if (suspended_while < greatest_suspend_for)
            {
              suspended_while = greatest_suspend_for;
              port->suspended_while_ticks = dot1x_to_ticks(suspended_while);
            }

          if (suspended_while < local_suspended_while)
            {
              suspended_while = local_suspended_while;
            }

          if (suspended_while == 0)
            dot1x_suspension_stop(port, "key server timeout");
          else
            dot1x_set_suspended_while(port, suspended_while);
        }
    }
  else
  if (greatest_suspend_for != DOT1X_MKA_IN_SERVICE_UPGRADE_UNSUPPORTED)
    {
      if (greatest_suspend_for > port->intf->ctrl.suspended_while)
        greatest_suspend_for = port->intf->ctrl.suspended_while;

      if (port->resuming_suspend && port->suspended_while > 0)
        {
          dot1x_suspension_start(
            port, mka, port->suspended_while, "local resumption");
        }
      else
      if (port->suspend_for)
        {
          dot1x_suspension_start(
            port, mka, greatest_suspend_for, "local suspension");
        }
      else
      if (port->suspend_on_request && greatest_suspend_for > 0)
        {
          dot1x_suspension_start(
            port, mka, greatest_suspend_for, "on request");
        }
    }
  else
    {
      dot1x_set_suspended_while(port, 0);
      set_can_suspend(port, 0);
    }
}

static void
step_suspension_non_keyserver(Dot1xMka *mka)
{
  Dot1xPort *port = mka->cak->port;
  Dot1xMkaKeySource *key_server = dot1x_get_key_server(port);

  if (is_suspended(port))
    {
      unsigned can_suspend = 0;

      if (port->suspend_for > 0 && port->suspend_for <= port->suspended_while)
        can_suspend = 1;

      set_can_suspend(port, can_suspend);

      if (key_server)
        {
          unsigned suspended_while = key_server->mka_peer->suspend_for;

          if (suspended_while == DOT1X_MKA_IN_SERVICE_UPGRADE_UNSUPPORTED)
            {
              dot1x_suspension_stop(port, "not supporting key server");
            }
          else
          if (suspended_while == 0)
            {
              dot1x_suspension_stop(port, "key server");
            }
          else
            {
              dot1x_set_suspended_while(port, suspended_while);
            }
        }
      else
        {
          unsigned suspended_while =
            dot1x_to_secs(port->local_suspended_while_ticks);

          if (suspended_while == 0)
            {
              dot1x_suspension_stop(port, "non key server timeout");
            }
          else
            {
              dot1x_set_suspended_while(
                      port,
                      suspended_while);
            }
        }
    }
  else
    {
      if (port->resuming_suspend && port->suspended_while > 0)
        {
          dot1x_suspension_start(
            port, mka, port->suspended_while,
            "local resumption of non-key server");
        }
      else
      if (key_server)
        {
          unsigned suspended_while = key_server->mka_peer->suspend_for;

          if (suspended_while != 0 &&
              suspended_while != DOT1X_MKA_IN_SERVICE_UPGRADE_UNSUPPORTED)
            {
              dot1x_suspension_start(
                port, mka, suspended_while, "non-key server");

              if (port->suspend_for <= suspended_while)
                {
                  set_can_suspend(port, 1);
                }
            }
        }
    }
}

static void
step_suspension(Dot1xMka *mka)
{
  if (is_local_key_server(mka))
    {
      step_suspension_keyserver(mka);
    }
  else
    {
      step_suspension_non_keyserver(mka);
    }

  /* Clear flags to only act once. */
  mka->suspension_unseen_peer = 0;
}

static int
is_suspended(Dot1xPort *port)
{
  return port->suspended;
}

static void
unref_suspended_peer(Dot1xPeer *peer)
{
  Dot1xPort *port = peer->port;

  if (peer->mka_reference_count <= 0 && peer->macsec_peer)
  {
    dot1x_debug_mka_secy_peer(peer, "release suspend");

    peer->mka_reference_count++;
    unref_peer(port, peer);
  }
}

void
dot1x_suspension_stop(Dot1xPort *port, const char *reason)
{
  dot1x_debug_port(port, "Stop suspension: %s.", reason);

  port->suspended = 0;

  dot1x_set_suspended_while(port, 0);
  set_can_suspend(port, 0);

  port->resuming_suspend = 0;

  foreach_peer(port, unref_suspended_peer);
}

void
dot1x_suspension_start(
  Dot1xPort *port,
  Dot1xMka *mka,
  unsigned suspended_while,
  const char *reason)
{
  dot1x_debug_port(port, "Start suspension: %s.", reason);
  port->suspended = 1;

  if (mka)
    mka->keyserver_suspend_ticks =
      dot1x_to_ticks(DOT1X_MKA_LIFE_TIME + DOT1X_MKA_HELLO_TIME);

  dot1x_set_suspended_while(port, suspended_while);
  port->suspended_while_ticks = dot1x_to_ticks(suspended_while);
  port->local_suspended_while_ticks = dot1x_to_ticks(suspended_while);
}

static void
update_chgd_server(Dot1xMka *mka)
{
  unsigned chgd_server = 0;

  if (is_local_key_server(mka) && !mka->cak->port->has_sa)
    {
      chgd_server = 1;
    }
  else
    {
      Dot1xMkaKeySource *source = mka->key_server;

      if (source &&
          source->mka_peer &&
          !source->mka_peer->peer->has_sa)
        chgd_server = 1;
    }

  set_chgd_server(mka->cak->port, chgd_server);
}
