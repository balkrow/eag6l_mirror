/**

   Public IEEE 802.1X-2010 functions.

   File: dot1xpae.c

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#include "dot1xpae.h"
#include "dot1xpae_pacp.h"
#include "dot1xpae_logon.h"
#include "dot1xpae_netann.h"
#include "dot1xpae_cp.h"
#include "dot1xpae_mka.h"
#include "dot1xpae_eapol.h"
#include "dot1xpae_util.h"
#include "eapauth.h"
#include "macseclmi.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

/*
 * Run the state machines of an interface to a stable state.
 */
static void
update_intf(Dot1xIntf *intf);

/*
 * Update all timers.
 */
static void
time_tick(Dot1xIntf *intf);

/*
 * Check if the state of the uncontrolled port has changed.
 */
static void
set_port_enabled(Dot1xIntf *intf);

/*
 * Process a frame received from the uncontrolled port.
 */
static void
process_frame(
  Dot1xIntf *intf, const MacsecAddress *dst, const MacsecAddress *src,
  unsigned char *msdu_buf, unsigned msdu_len);

/*
 * Process EAP conversation state associated with a PACP instance.
 */
static void
handle_eap(Dot1xPacp *pacp);

/*
 * Send pending packets, if any, produced by an EAP conversation
 * associated with a PACP.
 */
static void
send_eap_packets(Dot1xPacp *pacp);

static void
dot1x_macsec_peers_sync(Dot1xIntf *intf);

/*
 * Get real or virtual port status variables.
 */
static void
dot1x_get_port_stat(
  const Dot1xPort *port,
  Dot1xPortStat *stat);

/*
 * Default interface control values.
 */
static const Dot1xPaeCtrl default_pae_ctrl = {
  .eapol_group = DOT1X_EAPOL_GROUP_PAE,
  .virtual_ports_enable = 0,
  .announcer_enable = 0,
  .listener_enable = 0,
  .mka_enable = 1,
  .bounded_delay = 1,
  .actor_priority = 112,
  .cipher_suite = MACSEC_CIPHER_SUITE_GCM_AES_128,
  .confidentiality_offset = MACSEC_CONFIDENTIALITY_OFFSET_0,
  .logon = 1,
  .cak_caching_enable = 0,
  .group_ca_enable = 1,
  .long_eap_caks = 0,
  .supplicant_enable = 0,
  .authenticator_enable = 0,
  .held_period = 60,
  .quiet_period = 60,
  .reauth_enabled = 1,
  .reauth_period = 3600,
  .retry_max = 2,
  .logoff_enable = 1,
  .macsec_protect = 1,
  .macsec_validate = MACSEC_VALIDATE_FRAMES_STRICT,
  .macsec_replay_protect = 1,
  .transmit_delay = 6,
  .retire_delay = 3,
  .suspend_for = 0,
  .suspend_on_request = 0,
  .suspended_while = DOT1X_MKA_SUSPENSION_LIMIT
};

/*
 * Public functions.
 */

void
dot1x_init_intf(
  Dot1xIntf *intf,
  EapContext *eap_context,
  MacsecIntfId macsec_intf,
  const char *ifname)
{
  memset(intf, 0, sizeof *intf);

  strncpy(intf->ifname, ifname, sizeof intf->ifname);
  intf->ifname[sizeof intf->ifname - 1] = 0;

  dot1x_enable_message(intf, DOT1X_MESSAGE_ERROR);

  intf->eap_context = eap_context;
  intf->macsec_intf = macsec_intf;

  dot1x_crypto_random_init(&intf->crypto_random);

  /* set local MAC address */
  macsec_get_intf_attributes(
    intf->macsec_intf, &intf->local_ifindex, &intf->local_address);

  /* set control variables to default values */
  intf->ctrl = default_pae_ctrl;
  macsec_set_multicast_address(
    intf->macsec_intf, dot1x_eapol_address(intf->ctrl.eapol_group));

  /* set up supplicant and authenticator */
  intf->supp.intf = intf;
  dot1x_pacps_init(&intf->supp.pacps);
  intf->auth.intf = intf;
  dot1x_pacps_init(&intf->auth.pacps);

  /* set up default network */
  intf->dnetwork.intf = intf;
  intf->dnetwork.ctrl = dot1x_default_network_ctrl;
  macsec_list_init(&intf->dnetwork.pacps);
  macsec_list_init(&intf->dnetwork.caks);

  intf->next_vportid = 2;

  /* set up real port */
  intf->rport.intf = intf;
  intf->rport.portid = 1;
  macsec_list_init(&intf->rport.key_sources);

  /* init remaining collections */
  dot1x_networks_init(&intf->networks);
  dot1x_caks_init(&intf->caks);
  dot1x_mkas_init(&intf->mkas);
  dot1x_peers_init(&intf->peers);

  /* check peers from data plane */
  dot1x_macsec_peers_sync(intf);
}

void
dot1x_uninit_intf(
  Dot1xIntf *intf)
{
  dot1x_mkas_foreach(&intf->mkas, dot1x_destroy_mka);
  dot1x_pacps_foreach(&intf->supp.pacps, dot1x_destroy_pacp);
  dot1x_pacps_foreach(&intf->auth.pacps, dot1x_destroy_pacp);

  dot1x_crypto_random_uninit(&intf->crypto_random);

  memset(intf, 0, sizeof *intf);
}

void
dot1x_set_pae_ctrl(
  Dot1xIntf *intf,
  const Dot1xPaeCtrl *ctrl)
{
  Dot1xFormatBuffer fb;
  Dot1xNetwork *snetwork;

  if (ctrl->eapol_group != intf->ctrl.eapol_group)
    dot1x_debug_control(
      intf, "EAPOL group = %s", dot1x_eapol_group(ctrl->eapol_group));

  if (ctrl->virtual_ports_enable != intf->ctrl.virtual_ports_enable)
    dot1x_debug_control(
      intf, "virtualPortsEnable = %s", dot1x_bool(ctrl->virtual_ports_enable));

  if (ctrl->announcer_enable != intf->ctrl.announcer_enable)
    dot1x_debug_control(
      intf, "announcer.enable = %s", dot1x_bool(ctrl->announcer_enable));

  if (ctrl->listener_enable != intf->ctrl.listener_enable)
    dot1x_debug_control(
      intf, "listener.enable = %s", dot1x_bool(ctrl->listener_enable));

  if (ctrl->mka_enable != intf->ctrl.mka_enable)
    dot1x_debug_control(
      intf, "mka.enable = %s", dot1x_bool(ctrl->mka_enable));

  if (ctrl->bounded_delay != intf->ctrl.bounded_delay)
    dot1x_debug_control(
      intf, "bounded delay = %s", dot1x_bool(ctrl->bounded_delay));

  if (ctrl->actor_priority != intf->ctrl.actor_priority)
    dot1x_debug_control(
      intf, "actorPriority = %u", ctrl->actor_priority);

  if (ctrl->cipher_suite != intf->ctrl.cipher_suite)
    dot1x_debug_control(
      intf, "cipherSuite = %s", dot1x_cipher_suite(ctrl->cipher_suite));

  if (ctrl->confidentiality_offset != intf->ctrl.confidentiality_offset)
    dot1x_debug_control(
      intf, "confidentialityOffset = %s",
      dot1x_confidentiality_offset(ctrl->confidentiality_offset));

  if (ctrl->logon != intf->ctrl.logon)
    dot1x_debug_control(
      intf, "logon = %s", dot1x_bool(ctrl->logon));

  if (ctrl->selected_nid.len != intf->ctrl.selected_nid.len ||
      memcmp(ctrl->selected_nid.buf, intf->ctrl.selected_nid.buf,
             ctrl->selected_nid.len))
    {
      dot1x_debug_control(
        intf, "selectedNID = %s",
        dot1x_format_nid(&fb, &ctrl->selected_nid));
      snetwork = dot1x_networks_lookup(&intf->networks, &ctrl->selected_nid);
    }
  else
    {
      snetwork = intf->snetwork;
    }

  if (ctrl->cak_caching_enable != intf->ctrl.cak_caching_enable)
    dot1x_debug_control(
      intf, "CAK caching enable = %s", dot1x_bool(ctrl->cak_caching_enable));

  if (ctrl->long_eap_caks != intf->ctrl.long_eap_caks)
    dot1x_debug_control(
      intf, "long EAP CAKs = %s", dot1x_bool(ctrl->long_eap_caks));

  if (ctrl->supplicant_enable != intf->ctrl.supplicant_enable)
    dot1x_debug_control(
      intf, "supplicant.enable = %s", dot1x_bool(ctrl->supplicant_enable));

  if (ctrl->authenticator_enable != intf->ctrl.authenticator_enable)
    dot1x_debug_control(
      intf, "authenticator.enable = %s",
      dot1x_bool(ctrl->authenticator_enable));

  if (ctrl->held_period != intf->ctrl.held_period)
    dot1x_debug_control(
      intf, "heldPeriod = %u", ctrl->held_period);

  if (ctrl->quiet_period != intf->ctrl.quiet_period)
    dot1x_debug_control(
      intf, "quietPeriod = %u", ctrl->quiet_period);

  if (ctrl->reauth_enabled != intf->ctrl.reauth_enabled)
    dot1x_debug_control(
      intf, "reauthEnabled = %s", dot1x_bool(ctrl->reauth_enabled));

  if (ctrl->reauth_period != intf->ctrl.reauth_period)
    dot1x_debug_control(
      intf, "reauthPeriod = %u", ctrl->reauth_period);

  if (ctrl->retry_max != intf->ctrl.retry_max)
    dot1x_debug_control(
      intf, "retryMax = %u", ctrl->retry_max);

  if (ctrl->macsec_protect != intf->ctrl.macsec_protect)
    dot1x_debug_control(
      intf, "macsecProtect = %s", dot1x_bool(ctrl->macsec_protect));

  if (ctrl->macsec_validate != intf->ctrl.macsec_validate)
    dot1x_debug_control(
      intf, "macsecValidate = %s",
      dot1x_validate_frames(ctrl->macsec_validate));

  if (ctrl->macsec_replay_protect != intf->ctrl.macsec_replay_protect)
    dot1x_debug_control(
      intf, "macsecReplayProtect = %s",
      dot1x_bool(ctrl->macsec_replay_protect));

  if (ctrl->transmit_delay != intf->ctrl.transmit_delay)
    dot1x_debug_control(
      intf, "transmitDelay = %u", ctrl->transmit_delay);

  if (ctrl->retire_delay != intf->ctrl.retire_delay)
    dot1x_debug_control(
      intf, "retireDelay = %u", ctrl->retire_delay);

  if (ctrl->suspend_for != intf->ctrl.suspend_for)
    dot1x_debug_control(
      intf, "suspendFor = %u", ctrl->suspend_for);

  if (ctrl->suspend_on_request != intf->ctrl.suspend_on_request)
    dot1x_debug_control(
      intf, "suspendOnRequest = %s", dot1x_bool(ctrl->suspend_on_request));

  if (ctrl->suspended_while != intf->ctrl.suspended_while)
    dot1x_debug_control(
      intf, "suspendedWhile = %u", ctrl->suspended_while);


  intf->ctrl = *ctrl;
  macsec_set_multicast_address(
    intf->macsec_intf, dot1x_eapol_address(intf->ctrl.eapol_group));
  intf->snetwork = snetwork;
  intf->changed = 1;
  update_intf(intf);
}

void
dot1x_get_pae_ctrl(
  const Dot1xIntf *intf,
  Dot1xPaeCtrl *ctrl)
{
  *ctrl = intf->ctrl;
}

void
dot1x_get_pae_stat(
  const Dot1xIntf *intf,
  Dot1xPaeStat *stat)
{
  *stat = intf->stat;
}

void
dot1x_set_default_network_ctrl(
  Dot1xIntf *intf,
  Dot1xNetworkCtrl *ctrl)
{
  dot1x_set_network_ctrl(&intf->dnetwork, ctrl);
}

void
dot1x_get_default_network_ctrl(
  const Dot1xIntf *intf,
  Dot1xNetworkCtrl *ctrl)
{
  dot1x_get_network_ctrl(&intf->dnetwork, ctrl);
}

void
dot1x_get_default_network_stat(
  const Dot1xIntf *intf,
  Dot1xNetworkStat *stat)
{
  dot1x_get_network_stat(&intf->dnetwork, stat);
}

Dot1xNetwork *
dot1x_create_network(
  Dot1xIntf *intf,
  const Dot1xNid *nid)
{
  Dot1xNetwork *n;

  if ((n = dot1x_networks_lookup(&intf->networks, nid)))
    {
      if (n->life_ticks <= 0)
        {
          dot1x_error(intf, "network already managed");
          return NULL;
        }
    }
  else if (!(n = dot1x_insert_network(intf, nid)))
    {
      return NULL;
    }

  n->life_ticks = 0;
  return n;
}

void
dot1x_destroy_network(
  Dot1xNetwork *n)
{
  dot1x_remove_network(n);
}

Dot1xNetwork *
dot1x_next_network(
  const Dot1xIntf *intf,
  const Dot1xNetwork *prev)
{
  return dot1x_networks_next(&intf->networks, prev);
}

void
dot1x_get_network_attributes(
  const Dot1xNetwork *n,
  Dot1xNid *nid)
{
  memcpy(nid->buf, n->nid.buf, n->nid.len);
  nid->len = n->nid.len;
}

void
dot1x_set_network_ctrl(
  Dot1xNetwork *n,
  Dot1xNetworkCtrl *ctrl)
{
  Dot1xFormatBuffer fb;

  if (ctrl->use_eap != n->ctrl.use_eap)
    dot1x_debug_control_network(
      n, "useEAP = %s", dot1x_use_eap(ctrl->use_eap));

  if (ctrl->unauth_allowed != n->ctrl.unauth_allowed)
    dot1x_debug_control_network(
      n, "unauthAllowed = %s", dot1x_unauth_allowed(ctrl->unauth_allowed));

  if (ctrl->unsecured_allowed != n->ctrl.unsecured_allowed)
    dot1x_debug_control_network(
      n, "unsecuredAllowed = %s",
      dot1x_unsecured_allowed(ctrl->unsecured_allowed));

  if (dot1x_kmdcmp(&ctrl->kmd, &n->ctrl.kmd))
    dot1x_debug_control_network(
      n, "KMD = %s", dot1x_format_kmd(&fb , &ctrl->kmd));

  n->ctrl = *ctrl;
  n->intf->changed = 1;
  update_intf(n->intf);
}

void
dot1x_get_network_ctrl(
  const Dot1xNetwork *n,
  Dot1xNetworkCtrl *ctrl)
{
  *ctrl = n->ctrl;
}

void
dot1x_get_network_stat(
  const Dot1xNetwork *n,
  Dot1xNetworkStat *stat)
{
  *stat = n->stat;
}

Dot1xCak *
dot1x_create_cak(
  Dot1xIntf *intf,
  const Dot1xCkn *ckn,
  const MacsecKey *key,
  Dot1xCakType type,
  const MacsecAuthData *auth_data,
  const Dot1xNid *nid,
  void (*cak_activated_callback)(Dot1xCak*))
{
  Dot1xCak *cak;

  if (type != DOT1X_CAK_GROUP && type != DOT1X_CAK_PAIRWISE)
    {
      dot1x_error(intf, "invalid CAK type");
      return NULL;
    }

  if (ckn->len < 1 || ckn->len > sizeof ckn->buf)
    {
      dot1x_error(
        intf, "invalid CKN length %u octets, must be 1 to %u",
        key->len, (unsigned)(sizeof ckn->buf));
      return NULL;
    }

  if (key->len != 16 && key->len != 32)
    {
      dot1x_error(
        intf, "invalid CAK length %u octets, must be 16 or 32", key->len);
      return NULL;
    }

  if (!(cak = dot1x_insert_cak(intf, ckn, key, type, nid)))
    return NULL;
  dot1x_adcpy(&cak->auth_data, auth_data);

  cak->activated_callback = cak_activated_callback;
  intf->changed = 1;
  update_intf(intf);
  return cak;
}

Dot1xCak *
dot1x_insert_cak_cache(
        Dot1xIntf *intf,
        const Dot1xCak *cache_cak)
{
  Dot1xCak *cak;

  cak = dot1x_insert_cak(
    intf, &cache_cak->ckn, &cache_cak->key, cache_cak->type, &cache_cak->nid);
  if (cak)
    {
      cak->life_ticks = 0;
      cak->active_ticks = dot1x_to_ticks(DOT1X_MKA_LIFE_TIME);

      cak->pairwise       = cache_cak->pairwise;
      cak->preshared      = cache_cak->preshared;
      cak->supplicant     = cache_cak->supplicant;
      cak->authenticator  = cache_cak->authenticator;
      cak->generated      = cache_cak->generated;
      cak->enable         = cache_cak->enable;
      cak->eapfail        = cache_cak->eapfail;
      cak->has_partner    = cache_cak->has_partner;
      cak->partner        = cache_cak->partner;
      cak->kmd            = cache_cak->kmd;
      cak->auth_data      = cache_cak->auth_data;
      cak->parent_ckn     = cache_cak->parent_ckn;

      if (!cak->port)
      {
          if (cak->has_partner)
              cak->port = dot1x_refer_port(intf, &cak->partner);
          else
              cak->port = dot1x_refer_port(intf, NULL);
          if (!cak->port)
          {
              /* retry port creation after a while */
              cak->held_while_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
          }
          else
          {
              cak->port->authorization_data = &cak->auth_data;
              intf->logon_changed = 1;
          }
      }

      if (cak->type == DOT1X_CAK_EAP)
          dot1x_pacp_add_from_cache(intf, cak);

      if (!cak->port)
        {
          if (cak->has_partner)
              cak->port = dot1x_refer_port(intf, &cak->partner);
          else
              cak->port = dot1x_refer_port(intf, NULL);
          if (!cak->port)
            {
              /* retry port creation after a while */
              cak->held_while_ticks = dot1x_to_ticks(DOT1X_MKA_LIFE_TIME);
            }
          else
            {
              cak->port->authorization_data = &cak->auth_data;
            }
        }
    }

  return cak;
}

void
dot1x_destroy_cak(
  Dot1xCak *cak)
{
  Dot1xIntf *intf = cak->intf;

  dot1x_remove_cak(cak);
  intf->changed = 1;
  update_intf(intf);
}

Dot1xCak *
dot1x_next_cak(
  Dot1xIntf *intf,
  const Dot1xCak *prev)
{
  return dot1x_caks_next(&intf->caks, prev);
}

void
dot1x_get_cak_attributes(
  const Dot1xCak *cak,
  Dot1xCkn *ckn,
  Dot1xCakType *type,
  MacsecAuthData *auth_data,
  Dot1xNid *nid)
{
  dot1x_ckncpy(ckn, &cak->ckn);
  *type = cak->type;
  dot1x_adcpy(auth_data, &cak->auth_data);
  dot1x_nidcpy(nid, &cak->nid);
}

void
dot1x_get_cak_stat(
  const Dot1xCak *cak,
  Dot1xCakStat *stat)
{
  Dot1xMkaPeer *mkap;
  MacsecSci *lpt = stat->live_peer_tab;
  unsigned lpn, lps =
    sizeof stat->live_peer_tab / sizeof stat->live_peer_tab[0];
  unsigned ppn;
  MacsecSci *sci;

  dot1x_kmdcpy(&stat->kmd, &cak->kmd);
  stat->enabled = cak->enable;
  stat->lifetime = cak->life_ticks / DOT1X_TICKS_PER_SECOND;
  stat->activated = cak->sent_or_received_cak_or_sak;
  if (cak->port && cak->port->principal && cak->mka == cak->port->principal)
    stat->principal = 1;
  else
    stat->principal = 0;

  lpn = 0;
  ppn = 0;
  if (cak->mka)
    {
      mkap = NULL;
      lpn = 0;
      ppn = 0;
      while ((mkap = dot1x_mka_peers_next(&cak->mka->peers, mkap)))
        {
          if (mkap->live && lpn < lps)
            {
              sci = &lpt[lpn++];
              sci->address = mkap->peer->peer_address;
              sci->portid = mkap->peer->peer_portid;
            }
          else if (!mkap->live)
            ppn++;
          else
            break;

        }
    }
  stat->live_peer_num = lpn;
  stat->potential_peer_count = ppn;
}

Dot1xPort *
dot1x_next_virtual_port(
  Dot1xIntf *intf,
  const Dot1xPort *prev)
{
  Dot1xPeer *peer, *prevpeer;

  if (prev)
    prevpeer = (void *)((unsigned char *)prev - offsetof(Dot1xPeer, vport));
  else
    prevpeer = NULL;

  peer = dot1x_peers_next(&intf->peers, prevpeer);

  if (peer && peer->vport.portid)
    return &peer->vport;
  else
    return NULL;
}

void
dot1x_get_virtual_port_attributes(
  const Dot1xPort *port,
  unsigned *portid,
  MacsecAddress *peer_address)
{
  Dot1xPeer *peer;

  *portid = port->portid;

  peer = (void *)((unsigned char *)port - offsetof(Dot1xPeer, vport));
  *peer_address = peer->peer_address;
}

void
dot1x_get_real_port_stat(
  const Dot1xIntf *intf,
  Dot1xPortStat *stat)
{
  dot1x_get_port_stat(&intf->rport, stat);
}

void
dot1x_get_virtual_port_stat(
  const Dot1xPort *port,
  Dot1xPortStat *stat)
{
  dot1x_get_port_stat(port, stat);
}

void
dot1x_handle_events(
  Dot1xIntf *intf,
  const Dot1xEvents *events)
{
  unsigned char *msdu_buf = intf->msdu;
  unsigned msdu_max = sizeof intf->msdu, msdu_len;
  MacsecAddress dst, src;

  if (events->time_tick)
    time_tick(intf);

  if (events->common_port_state)
    set_port_enabled(intf);

  if (events->uncontrolled_port_receive)
    {
      while ((msdu_len = macsec_receive_uncontrolled(
                intf->macsec_intf, &dst, &src, msdu_buf, msdu_max)))
        process_frame(intf, &dst, &src, msdu_buf, msdu_len);
    }

  if (events->eap_state)
    {
      dot1x_pacps_foreach(&intf->supp.pacps, handle_eap);
      dot1x_pacps_foreach(&intf->auth.pacps, handle_eap);
    }

  update_intf(intf);
}

void
dot1x_enable_message(
  Dot1xIntf *intf,
  Dot1xMessage msg)
{
  uint32_t bit = 1 << msg;

  if ((intf->messages & bit))
    return;

  intf->messages |= bit;
}

void
dot1x_disable_message(
  Dot1xIntf *intf,
  Dot1xMessage msg)
{
  uint32_t bit = 1 << msg;

  if (!(intf->messages & bit))
    return;

  intf->messages &= ~bit;
}

int
dot1x_test(void)
{
  int ok = 1;

  ok &= dot1x_test_logon();
  ok &= dot1x_test_mka();
  return ok;
}

/*
 * Static functions.
 */

static void
update_intf(Dot1xIntf *intf)
{
  int cycles = 0;

  while (intf->changed)
    {
      if (cycles++ > 10)
        {
          dot1x_error(intf, "interface did not stabilize after 10 cycles");
          break;
        }

      intf->changed = 0;
      intf->changed |= dot1x_step_netann(intf);
      intf->changed |= dot1x_step_logon(intf);
      intf->changed |= dot1x_step_pacp(intf);
      intf->changed |= dot1x_step_mka(intf);

      if  (intf->changed)
        continue;

      intf->changed |= dot1x_step_cp(intf);
    }

  dot1x_send_mka(intf);
}

static void
time_tick(
  Dot1xIntf *intf)
{
  dot1x_tick_pacp(intf);
  dot1x_tick_netann(intf);
  dot1x_tick_logon(intf);
  dot1x_tick_mka(intf);
  dot1x_tick_cp(intf);
  intf->changed = 1;
}

static void
set_port_enabled(
  Dot1xIntf *intf)
{
  int value;

  macsec_get_common_port_enabled(intf->macsec_intf, &value);

  if (intf->port_enabled == value)
    return;

  intf->port_enabled = value;

  dot1x_debug_cp_secy(
    &intf->rport, "portEnabled = %s", dot1x_bool(intf->port_enabled));
  intf->changed = 1;
}

static void
process_frame(
  Dot1xIntf *intf, const MacsecAddress *dst, const MacsecAddress *src,
  unsigned char *msdu_buf, unsigned msdu_len)
{
  unsigned version, type;
  const unsigned char *buf;
  unsigned len;
  Dot1xFormatBuffer fb;

  if (memcmp(dst, dot1x_eapol_address(intf->ctrl.eapol_group), sizeof *dst) &&
      memcmp(dst, &intf->local_address, sizeof *dst))
    {
      dot1x_debug_discard(intf, "received MAC frame to wrong address");
      return;
    }

  if ((src->buf[0] & 1))
    {
      dot1x_debug_discard(intf, "received MAC frame from multicast address");
      return;
    }

  if (msdu_len < DOT1X_EAPOL_HEADER_SIZE)
    {
      dot1x_debug_discard(intf, "EAPOL frame too short");
      return;
    }

  if (dot1x_decode_eapol_header(msdu_buf, &version, &type, &len) < 0)
    {
      dot1x_debug_discard(intf, "not EAPOL ethertype");
      return;
    }

  intf->stat.last_eapol_frame_source = *src;
  intf->stat.last_eapol_frame_version = version;

  if (version < 1)
    {
      dot1x_debug_discard(intf, "invalid EAPOL version %u", version);
      return;
    }

  if (DOT1X_EAPOL_HEADER_SIZE + len > msdu_len)
    {
      dot1x_debug_discard(intf, "truncated EAPOL PDU");
      intf->stat.eap_length_error_frames++;
      return;
    }

  buf = msdu_buf + DOT1X_EAPOL_HEADER_SIZE;

  dot1x_debug_eapol(
    intf, "received EAPOL-%s from %s",
    dot1x_eapol_type(type), dot1x_format_address(&fb, src));

  switch (type)
    {
    case DOT1X_EAPOL_START:
      dot1x_receive_start(intf, version, src, buf, len);
      break;

    case DOT1X_EAPOL_LOGOFF:
      dot1x_receive_logoff(intf, src);
      break;

    case DOT1X_EAPOL_EAP:
      dot1x_receive_eap_msg(intf, dst, src, buf, len);
      break;

    case DOT1X_EAPOL_MKA:
      /* MKA needs the whole MSDU for ICV calculation */
      dot1x_receive_mkpdu(intf, dst, src, msdu_buf, msdu_len);
      break;

    case DOT1X_EAPOL_ANNOUNCEMENT_REQ:
      dot1x_receive_announcement_req(intf, src, buf, len);
      break;

    case DOT1X_EAPOL_ANNOUNCEMENT_GEN:
    case DOT1X_EAPOL_ANNOUNCEMENT_SPC:
      dot1x_receive_announcement(intf, dst, type, buf, len);
      break;
    default:
      dot1x_debug_discard(
        intf, "received unsupported EAPOL packet type %u", type);
      intf->stat.invalid_eapol_frames++;
      break;
    }
}

static void
handle_eap(Dot1xPacp *pacp)
{
  EapConversation *c = pacp->eap_conversation;

  if (!c)
    return;

  switch (eap_state(c))
    {
    case EAP_STATE_SUCCEEDED:
      if (!pacp->eap_success)
        {
          pacp->eap_success = 1;
          dot1x_debug_pacp_eap(
            pacp, "eapSuccess = %s", dot1x_bool(pacp->eap_success));
          pacp->side->intf->changed = 1;
        }
      break;

    case EAP_STATE_FAILED:
      if (!pacp->eap_fail)
        {
          pacp->eap_fail = 1;
          dot1x_debug_pacp_eap(
            pacp, "eapFail = %s", dot1x_bool(pacp->eap_fail));
          pacp->side->intf->changed = 1;
        }
      break;

    case EAP_STATE_TIMEDOUT:
      if (!pacp->eap_timeout)
        {
          pacp->eap_timeout = 1;
          dot1x_debug_pacp_eap(
            pacp, "eapTimeout = %s", dot1x_bool(pacp->eap_timeout));
          pacp->side->intf->changed = 1;
        }
      break;

    default:
      send_eap_packets(pacp);
      break;
    }
}

static void
send_eap_packets(Dot1xPacp *pacp)
{
  Dot1xIntf *intf = pacp->side->intf;
  const MacsecAddress *dst, *src;
  const MacsecAddress zero = {.buf = {0, 0, 0, 0, 0, 0}};
  unsigned char *msdu_buf = intf->msdu, *buf;
  unsigned msdu_max = sizeof intf->msdu, msdu_len, max, len;
  Dot1xFormatBuffer fb;

  buf = msdu_buf + DOT1X_EAPOL_HEADER_SIZE;
  max = msdu_max - DOT1X_EAPOL_HEADER_SIZE;

  while (1)
    {

      if ((len = eap_produce_packet(pacp->eap_conversation, buf, max)) <= 0)
        break;

      dot1x_encode_eapol_header(msdu_buf, 3, DOT1X_EAPOL_EAP, len);
      msdu_len = DOT1X_EAPOL_HEADER_SIZE + len;

      if (memcmp(&pacp->partner, &zero, sizeof pacp->partner))
        dst = &pacp->partner;
      else
        dst = dot1x_eapol_address(intf->ctrl.eapol_group);

      src = &intf->local_address;

      dot1x_debug_eap(
        intf, "sending EAP packet to %s", dot1x_format_address(&fb, dst));

      if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_EAP) &&
          dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
        dot1x_print_hex(intf, buf, len);

      dot1x_debug_eapol(
        intf, "sending EAPOL-EAP packet to %s",
        dot1x_format_address(&fb, dst));

      if (pacp->side == &intf->supp)
        intf->stat.eapol_supp_eap_frames_tx++;
      else
        intf->stat.eapol_auth_eap_frames_tx++;

      if (!macsec_send_uncontrolled(
            intf->macsec_intf, dst, src, msdu_buf, msdu_len))
        dot1x_error(intf, "cannot send to uncontrolled port");
    }
}

static void
dot1x_get_port_stat(
  const Dot1xPort *port,
  Dot1xPortStat *stat)
{
  Dot1xMkaKeySource *ks;

  if (!macsec_list_empty(&port->key_sources))
    stat->mka_active = 1;
  else
    stat->mka_active = 0;

  stat->mka_failed = port->mka_failed;
  stat->mka_authenticated = port->mka_authenticated;
  stat->mka_secured = port->mka_secured;

  if ((ks = dot1x_get_key_server(port)))
    {
      memcpy(
        &stat->key_server_sci.address,
        ks->sci, sizeof stat->key_server_sci.address);
      stat->key_server_sci.portid = ks->sci[6] << 8 | ks->sci[7];
      stat->key_server_priority = ks->priority;
    }
  else
    {
      memset(&stat->key_server_sci, 0, sizeof stat->key_server_sci);
      stat->key_server_priority = 0;
    }

  stat->kn = port->distributed_ki.kn;
  stat->an = port->distributed_an;
}

static void
dot1x_create_peer(
  Dot1xIntf *intf,
  MacsecPeerId peer_id,
  unsigned portid,
  MacsecAddress *address,
  unsigned peer_portid)
{
  Dot1xPeerType type = DOT1X_PEER_REAL;
  Dot1xPeer *peer;

  if (portid != 1)
    type = DOT1X_PEER_VIRTUAL;

  peer = dot1x_refer_peer_portid(intf, type, address, portid);
  if (peer)
    {
      Dot1xPort *port = peer->port;
      unsigned latest_an;

      peer->peer_portid = peer_portid;
      peer->macsec_peer = peer_id;
      peer->has_sa = 1;

      macsec_get_latest_an(intf->macsec_intf, portid, &latest_an);

      /* keep track of per-port highest live peer station count */
      port->peer_count++;
      if (port->peer_count > port->highest_peer_count)
          port->highest_peer_count = port->peer_count;

      port->transmit_sa_an = latest_an;

      /* set up the AN to use after the next SAK change */
      port->next_an = latest_an + 1;
      port->next_an &= 3;

      port->resuming_suspend = 1;
      port->has_sa = 1;
    }
}

static void
dot1x_macsec_peers_sync(Dot1xIntf *intf)
{
  Dot1xFormatBuffer fb;

  MacsecPeerId peer_id = 0;

  dot1x_error(intf, "Checking peers!");

  while ((peer_id = macsec_next_peer(intf->macsec_intf, peer_id)))
    {
      unsigned portid;
      MacsecAddress peer_address;
      unsigned peer_portid;

      macsec_get_peer_attributes(
        peer_id, &portid, &peer_address, &peer_portid);

      dot1x_error(
        intf, "Got peer %u %s %u", portid,
        dot1x_format_address(&fb, &peer_address), peer_portid);

      dot1x_create_peer(intf, peer_id, portid, &peer_address, peer_portid);
    }
}

void
dot1x_set_cak_cache_callback(
        Dot1xIntf *intf,
        void (*cak_cache_callback)(Dot1xCakCacheOp op, const Dot1xCak *cak))
{
  intf->cak_cache_callback = cak_cache_callback;
}
