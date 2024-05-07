/**

   IEEE 802.1X-2010 PACP state machines.

   File: dot1xpae_pacp.c

   @description
   The step functions of the machines are mechanical transcriptions of
   Figures 8.6 and 8.7 of 802.1X-2010; each arrow of the figure
   corresponds to a group of conditions in an `if' expression, each
   group consisting of an input state condition and potentially some
   input variable conditions. Each box of the figure corresponds to a
   block of statements guarded by an `if' expression.

   Differences from Figure 8.6:

   - In the transition from INITIALIZE to UNAUTHENTICATED, require
     portEnabled in addition to the other conditions.

   - The variables supp.eapStop and supp.eapStart are not explicitly
     modeled. Instead, the functions eap_destroy() and
     eap_create_authenticator/supplicant() are called when
     appropriate. Whenever supp.eapStop appears as a condition of a
     transition it is replaced by FALSE.

   - Add transition from AUTHENTICATED to AUTHENTICATING on reception
     of a new message (reauthentication), and instead of sending
     EAPOL-Start, just process the message. This transition is done in
     receive_msg_supp(), not in the step function.

   - In case of CAK creation failure after entering AUTHENTICATED,
     move to HELD.

   - Add transition from UNAUTHENTICATED or AUTHENTICATED to
     INITIALIZE if the network associated with the PACP has been
     destroyed.

   Differences from figure 8.7:

   - In the transitions from INITIALIZE to UNAUTHENTICATED and from
     INITIALIZE to INITIALIZE, require portEnabled in addition to the
     other conditions.

   - The variables auth.eapStop and auth.eapStart are not explicitly
     modeled. Instead, the functions eap_destroy() and
     eap_create_authenticator/supplicant() are called when
     appropriate. Whenever auth.eapStop appears as a condition of a
     transition it is replaced by FALSE.

   - Add transition from AUTHENTICATED to AUTHENTICATING on
     eapFail. This is to support artificial eapFail generated if the
     EAP-based CAK is removed while the PACP is authenticated. Similar
     artificial eapFail is generated in the supplicant but it already
     has this transition.

   - In case of CAK creation failure after entering AUTHENTICATED,
     move to HELD.

   - Add transition from UNAUTHENTICATED or AUTHENTICATED to
     INITIALIZE if the network associated with the PACP has been
     destroyed.

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#include "dot1xpae_pacp.h"
#include "dot1xpae_eapol.h"
#include "dot1xpae_logon.h"
#include "dot1xpae_netann.h"
#include "dot1xpae_anpdu.h"
#include "dot1xpae_util.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>

/*
 * Run the state machine of a supplicant PACP instance.
 */
static void
step_instance_supp(Dot1xPacp *pacp);

/*
 * Run the state machine of an authenticator PACP instance.
 */
static void
step_instance_auth(Dot1xPacp *pacp);

/*
 * Advance the timers of an authenticator PACP instance.
 */
static void
tick_instance_auth(Dot1xPacp *pacp);

/*
 * Advance the timers of a supplicant PACP instance.
 */
static void
tick_instance_supp(Dot1xPacp *pacp);

/*
 * Handle an incoming EAP message to a supplicant.
 */
static void
receive_msg_supp(
  Dot1xIntf *intf, const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *msg_buf, unsigned msg_len);

/*
 * Handle an incoming EAP message to an authenticator.
 */
static void
receive_msg_auth(
  Dot1xIntf *intf, const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *msg_buf, unsigned msg_len);

/*
 * Send an EAPOL-Start packet.
 */
static void
send_start(Dot1xPacp *pacp);

/*
 * Send an EAPOL-Logoff packet.
 */
static void
send_logoff(Dot1xPacp *pacp);

/*
 * Set PACP variables.
 */
static void
set_state(Dot1xPacp *pacp, int value);
static void
set_enabled(Dot1xPacp *pacp, int value);
static void
set_authenticated(Dot1xPacp *pacp, int value);
static void
get_results(Dot1xPacp *pacp);
static void
set_failed(Dot1xPacp *pacp, int value);
static void
eap_stop(Dot1xPacp *pacp);
static unsigned
eap_start(Dot1xPacp *pacp);
static void
set_eap_success(Dot1xPacp *pacp, int value);
static void
set_eap_timeout(Dot1xPacp *pacp, int value);
static void
set_eap_fail(Dot1xPacp *pacp, int value);
static void
set_rx_eapol_start(Dot1xPacp *pacp, int value);
static void
set_rx_eapol_logoff(Dot1xPacp *pacp, int value);
static void
set_retry_count(Dot1xPacp *pacp, unsigned value);
static void
set_held_while(Dot1xPacp *pacp, unsigned value);
static void
set_quiet_while(Dot1xPacp *pacp, unsigned value);
static void
set_reauth_when(Dot1xPacp *pacp, unsigned value);

/*
 * Try rebinding a PACP instance identified by the its partner MAC
 * address with a new partner MAC address.
 */
static Dot1xPacp *
rebind(
  Dot1xPacpSide *side, const MacsecAddress *oldpart,
  const MacsecAddress *newpart);

/*
 * Zero MAC address to indicate no EAP partner.
 */
static const MacsecAddress zero;

/*
 * Public functions.
 */

void
dot1x_link_pacp_network(Dot1xPacp *pacp, Dot1xNetwork *netw)
{
  Dot1xFormatBuffer fb;

  pacp->network = netw;
  macsec_list_append(&netw->pacps, &pacp->network_link);
  dot1x_debug_pacp(
    pacp, "associated with %s", dot1x_format_network(&fb, netw));
}

void
dot1x_unlink_pacp_network(Dot1xPacp *pacp)
{
  macsec_list_remove(&pacp->network_link);
  pacp->network = NULL;
}

void
dot1x_link_pacp_cak(Dot1xPacp *pacp, Dot1xCak *cak)
{
  pacp->cak = cak;
  macsec_list_append(&cak->pacps, &pacp->cak_link);
}

void
dot1x_unlink_pacp_cak(Dot1xPacp *pacp)
{
  macsec_list_remove(&pacp->cak_link);
  pacp->cak = NULL;
}

Dot1xPacp *
dot1x_create_pacp(Dot1xPacpSide *side, const MacsecAddress *partner)
{
  Dot1xIntf *intf = side->intf;
  Dot1xPacp *pacp;
  Dot1xFormatBuffer fb;

  if (!(pacp = dot1x_pacps_insert(&side->pacps, partner)))
    {
      dot1x_error(intf, "out of %s PACP instances", dot1x_pacp_side(side));
      goto fail;
    }
  pacp->side = side;

  if (!memcmp(partner, &zero, sizeof *partner))
    pacp->port = dot1x_refer_port(intf, NULL);
  else
    pacp->port = dot1x_refer_port(intf, partner);
  if (!pacp->port)
    goto fail;

  dot1x_debug_pacp(pacp, "created");

  if (memcmp(partner, &zero, sizeof *partner))
    dot1x_debug_pacp(
      pacp, "associated with partner %s",
      dot1x_format_address(&fb, &pacp->partner));

  return pacp;

 fail:
  dot1x_destroy_pacp(pacp);
  return NULL;
}

void
dot1x_destroy_pacp(Dot1xPacp *pacp)
{
  Dot1xPacpSide *side;

  if (!pacp)
    return;

  side = pacp->side;

  dot1x_debug_pacp(pacp, "being destroyed");

  if (pacp->cak)
    dot1x_unlink_pacp_cak(pacp);

  if (pacp->eap_conversation)
    eap_stop(pacp);

  if (pacp->network)
    dot1x_unlink_pacp_network(pacp);

  set_enabled(pacp, 0);
  set_authenticated(pacp, 0);

  if (pacp->port)
    {
      dot1x_unref_port(pacp->port);
      pacp->port = NULL;
    }

  assert(dot1x_pacps_remove(&side->pacps, &pacp->partner) == pacp);
  memset(pacp, 0, sizeof *pacp);
}

int
dot1x_step_pacp(Dot1xIntf *intf)
{
  unsigned supp_allowed, auth_allowed;

  /* determine whether or not PACPs should exist */
  supp_allowed =
    intf->port_enabled &&
    intf->ctrl.supplicant_enable &&
    intf->rport.supp_authenticate;
  auth_allowed =
    intf->port_enabled &&
    intf->ctrl.authenticator_enable &&
    intf->rport.auth_authenticate;

  /* create default supplicant PACP if approriate, none exists,
  virtual ports disabled and we are not already authenticated as
  either supplicant or authenticator */
  if (supp_allowed &&
      dot1x_pacps_count(&intf->supp.pacps) <= 0 &&
      !intf->ctrl.virtual_ports_enable &&
      !intf->supp.authenticated && !intf->auth.authenticated)
    {
      dot1x_create_pacp(&intf->supp, &zero);
      /* does not fail */
    }

  /* create default authenticator PACP if appropriate, none exists,
     virtual ports disabled and we are not already authenticated as
     either supplicant or authenticator */
  if (auth_allowed &&
      dot1x_pacps_count(&intf->auth.pacps) <= 0 &&
      !intf->ctrl.virtual_ports_enable &&
      !intf->supp.authenticated && !intf->auth.authenticated)
    {
      dot1x_create_pacp(&intf->auth, &zero);
      /* does not fail */
    }

  dot1x_pacps_foreach(&intf->supp.pacps, step_instance_supp);
  dot1x_pacps_foreach(&intf->auth.pacps, step_instance_auth);

  if (intf->pacp_changed)
    {
      intf->pacp_changed = 0;
      return 1;
    }

  return 0;
}

void
dot1x_tick_pacp(Dot1xIntf *intf)
{
  dot1x_pacps_foreach(&intf->supp.pacps, tick_instance_supp);
  dot1x_pacps_foreach(&intf->auth.pacps, tick_instance_auth);
}

void
dot1x_receive_eap_msg(
  Dot1xIntf *intf, const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *buf, unsigned len)
{
  Dot1xFormatBuffer fb;

  if (len < 1)
    {
      dot1x_error(intf, "empty EAP message");
      return;
    }

  dot1x_debug_eap(
    intf, "received EAP packet from %s", dot1x_format_address(&fb, src));

  if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_EAP) &&
      dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
    dot1x_print_hex(intf, buf, len);

  /* send EAP responses (code 2) to a local authenticator */
  /* send other codes to a local supplicant */
  if (buf[0] == 2)
    {
      intf->stat.eapol_auth_eap_frames_rx++;
      receive_msg_auth(intf, dst, src, buf, len);
    }
  else
    {
      intf->stat.eapol_supp_eap_frames_rx++;
      receive_msg_supp(intf, dst, src, buf, len);
    }
}

void
dot1x_receive_start(
  Dot1xIntf *intf, unsigned version, const MacsecAddress *src,
  const unsigned char *buf, unsigned len)
{
  Dot1xAnpduReq *req = &intf->anpdureq;
  Dot1xFormatBuffer fb;
  Dot1xPacp *pacp;

  dot1x_debug_eap(
    intf, "received EAP start from %s", dot1x_format_address(&fb, src));

  intf->stat.eapol_start_frames_rx++;

  if (!intf->ctrl.authenticator_enable)
    return;

  /* find an authenticator with matching partner MAC address, take
     the authenticator without partner, i.e. the default
     authenticator, or start a new authenticator */
  if ((pacp = dot1x_pacps_lookup(&intf->auth.pacps, src)))
    ;
  else if ((pacp = rebind(&intf->auth, &zero, src)))
    ;
  else if ((pacp = dot1x_create_pacp(&intf->auth, src)))
    ;
  else
    return;

  /* handle embedded announcement request in version 3 EAPOL-Start */
  if (version >= 3 && len > 0)
    {
      /* it's a solicit if LSB of first byte is set */
      if ((buf[0] & 1))
        pacp->port->announcement_solicited = 1;

      if (!dot1x_decode_announcement_req(intf, req, buf + 1, len - 1))
        {
          dot1x_debug_discard(intf, "cannot decode announcement request");
          goto reqend;
        }
      if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_EAP) &&
          dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
        {
          dot1x_printf(intf, "    Request = %u\n", buf[0] & 1);
          dot1x_print_announcement_req(intf, req);
        }
      dot1x_process_announcement_req(pacp->port, req);
    }
 reqend:

  set_rx_eapol_start(pacp, 1);
  intf->changed = 1;
}

void
dot1x_receive_logoff(Dot1xIntf *intf, const MacsecAddress *src)
{
  Dot1xPacp *pacp;
  Dot1xFormatBuffer fb;

  dot1x_debug_eap(
    intf, "received EAP logoff from %s", dot1x_format_address(&fb, src));

  intf->stat.eapol_logoff_frames_rx++;

  if (!intf->ctrl.authenticator_enable)
    return;

  if (!intf->ctrl.logoff_enable)
    {
      dot1x_debug_discard(
        intf, "ignoring EAP logoff from %s", dot1x_format_address(&fb, src));
      return;
    }

  /* find an authenticator with matching partner MAC address */
  if (!(pacp = dot1x_pacps_lookup(&intf->auth.pacps, src)))
    {
      dot1x_debug_discard(
        intf, "unexpected EAP logoff from %s",
        dot1x_format_address(&fb, src));
      return;
    }

  set_rx_eapol_logoff(pacp, 1);
  intf->changed = 1;
}

/*
 * Static functions.
 */

static void
step_instance_supp(Dot1xPacp *pacp)
{
  Dot1xIntf *intf = pacp->side->intf;
  Dot1xNetwork *netw = pacp->network;

  /* if the CAK of a successfully authenticated PACP been removed set
     eapFail, or if the authenticator side is authenticated remove
     the PACP */
  if ((pacp->state == DOT1X_PACP_AUTHENTICATED &&
       !pacp->cak &&
       netw->ctrl.unsecured_allowed != DOT1X_UNSECURED_ALLOWED_IMMEDIATE &&
       netw->ctrl.unsecured_allowed != DOT1X_UNSECURED_ALLOWED_MKA_FAIL))
    {
      if (intf->auth.authenticated)
        {
          dot1x_destroy_pacp(pacp);
          intf->pacp_changed = 1;
          return;
        }
      set_eap_fail(pacp, 1);
    }

 begin:
  if (pacp->state == DOT1X_PACP_BEGIN ||

      (pacp->state != DOT1X_PACP_INITIALIZE &&
       (!intf->port_enabled ||
        (pacp->enabled && !intf->ctrl.supplicant_enable))) ||

      ((pacp->state == DOT1X_PACP_AUTHENTICATING || /* extra */
        pacp->state == DOT1X_PACP_AUTHENTICATED) &&
       (!netw || (intf->snetwork && netw != intf->snetwork))))
    {
      set_state(pacp, DOT1X_PACP_INITIALIZE);
      set_enabled(pacp, 0);
      set_authenticated(pacp, 0);
      set_failed(pacp, 0);
      eap_stop(pacp);
      set_retry_count(pacp, 0);
      intf->pacp_changed = 1;
      goto begin; /* re-run for potential 2nd transition to UNAUTHENTICATED */
    }
  else
  if ((pacp->state == DOT1X_PACP_AUTHENTICATING &&
       !pacp->port->supp_authenticate &&
       (intf->stat.supp_eap_logoff_while_authenticating++ || 1)) ||

      (pacp->state == DOT1X_PACP_AUTHENTICATED &&
       (!pacp->port->supp_authenticate || !intf->ctrl.supplicant_enable) &&
       (intf->stat.supp_auth_eap_logoff_while_authenticated++ || 1)))
    {
      set_state(pacp, DOT1X_PACP_LOGOFF);
      eap_stop(pacp);
      send_logoff(pacp);
      intf->pacp_changed = 1;
    }
  else
  if ((pacp->state == DOT1X_PACP_AUTHENTICATING &&
       pacp->eap_fail &&
       (intf->stat.supp_auth_fail_while_authenticating++ || 1)))
    {
    held:
      set_state(pacp, DOT1X_PACP_HELD);
      set_failed(pacp, 1);
      set_authenticated(pacp, 0);
      set_held_while(pacp, intf->ctrl.held_period);
      intf->pacp_changed = 1;
    }
  else
  if ((pacp->state == DOT1X_PACP_INITIALIZE &&
       intf->port_enabled /* extra */ &&
       intf->ctrl.supplicant_enable) ||

      pacp->state == DOT1X_PACP_LOGOFF ||

      (pacp->state == DOT1X_PACP_HELD &&
       pacp->held_while_ticks == 0) ||

      (pacp->state == DOT1X_PACP_AUTHENTICATING &&
       pacp->eap_timeout &&
       pacp->retry_count >= intf->ctrl.retry_max &&
       (intf->stat.supp_auth_timeouts_while_authenticating++ || 1)))
    {
      set_state(pacp, DOT1X_PACP_UNAUTHENTICATED);
      set_enabled(pacp, 1);
      set_authenticated(pacp, 0);
      set_failed(
        pacp, pacp->failed || pacp->retry_count >= intf->ctrl.retry_max);
      eap_stop(pacp); /* extra */
      set_retry_count(pacp, 0);
      intf->pacp_changed = 1;
    }
  else
  if ((pacp->state == DOT1X_PACP_UNAUTHENTICATED &&
       pacp->port->supp_authenticate &&
       !pacp->failed &&
       /* !supp.eapStop */
       (intf->stat.supp_enters_authenticating++ || 1)) ||

      (pacp->state == DOT1X_PACP_AUTHENTICATING &&
       pacp->eap_timeout &&
       pacp->retry_count < intf->ctrl.retry_max &&
       (intf->stat.supp_auth_timeouts_while_authenticating++ || 1)) ||

      (pacp->state == DOT1X_PACP_AUTHENTICATED &&
       pacp->eap_fail &&
       (intf->stat.supp_auth_fail_while_authenticated++ || 1)))
    {
      set_state(pacp, DOT1X_PACP_AUTHENTICATING);
      set_eap_success(pacp, 0);
      set_eap_timeout(pacp, 0);
      set_eap_fail(pacp, 0);
      if (!eap_start(pacp))
        set_eap_fail(pacp, 1);
      else
        send_start(pacp);
      set_retry_count(pacp, pacp->retry_count + 1);
      intf->pacp_changed = 1;
    }
  else
  if ((pacp->state == DOT1X_PACP_AUTHENTICATING &&
       pacp->eap_success &&
       (intf->stat.supp_auth_successes_while_authenticating++ || 1)))
    {
      set_state(pacp, DOT1X_PACP_AUTHENTICATED);
      set_authenticated(pacp, 1);
      get_results(pacp);
      /* on failed CAK creation shortcut to HELD */
      if (!pacp->cak &&
          netw->ctrl.unsecured_allowed != DOT1X_UNSECURED_ALLOWED_IMMEDIATE &&
          netw->ctrl.unsecured_allowed != DOT1X_UNSECURED_ALLOWED_MKA_FAIL)
        goto held;
      set_retry_count(pacp, 0);
      intf->pacp_changed = 1;
    }

  /* remove inactive or failed PACP instance */
  if (pacp->state == DOT1X_PACP_INITIALIZE ||

      (pacp->state == DOT1X_PACP_UNAUTHENTICATED &&
       (!pacp->port->supp_authenticate || pacp->failed)))
    {
      dot1x_destroy_pacp(pacp);
      intf->pacp_changed = 1;
    }
}

static unsigned
match_network(Dot1xNetwork *network, Dot1xNid *nid)
{
  return network != NULL && memcmp(&network->nid, nid, sizeof *nid) == 0;
}

Dot1xNetwork *
dot1x_choose_network_by_nid(Dot1xPort *port, Dot1xNid *nid)
{
  Dot1xIntf *intf = port->intf;

  if (match_network(intf->snetwork, nid))
    return intf->snetwork;
  else if (match_network(port->rnetwork, nid))
    return port->rnetwork;
  else if (match_network(port->cnetwork, nid))
    return port->cnetwork;
  else
    return &intf->dnetwork;
}

void
dot1x_pacp_add_from_cache(Dot1xIntf *intf, Dot1xCak *cak)
{
  Dot1xPacpSide *side = NULL;
  Dot1xPacp *pacp = NULL;

  if (cak->supplicant)
    {
      side = &intf->supp;
      dot1x_debug_logon_pacp(intf, "Create supplicant from cached CAK");
    }
  else
  if (cak->authenticator)
    {
      side = &intf->auth;
      dot1x_debug_logon_pacp(intf, "Create authenticator from cached CAK");
    }
  else
    {
      dot1x_debug_logon_pacp(intf, "Create none from cached CAK");
    }

  if (side)
    {
      pacp = dot1x_create_pacp(side, &cak->partner);
    }

  if (pacp)
    {
      dot1x_link_pacp_cak(pacp, cak);

      if (cak->port)
        {
          dot1x_link_pacp_network(
                  pacp, dot1x_choose_network_by_nid(cak->port, &cak->nid));
        }

      set_enabled(pacp, 1);
      set_state(pacp, DOT1X_PACP_AUTHENTICATED);
      set_authenticated(pacp, 1);

      set_retry_count(pacp, 0);
      set_reauth_when(pacp, intf->ctrl.reauth_period);
      intf->pacp_changed = 1;
    }
}

static void
step_instance_auth(Dot1xPacp *pacp)
{
  Dot1xIntf *intf = pacp->side->intf;
  Dot1xNetwork *netw = pacp->network;

  /* if the CAK of a successfully authenticated PACP been removed set
     eapFail, or if the supplicant side is authenticated remove
     the PACP */
  if ((pacp->state == DOT1X_PACP_AUTHENTICATED &&
       !pacp->cak &&
       netw->ctrl.unsecured_allowed != DOT1X_UNSECURED_ALLOWED_IMMEDIATE &&
       netw->ctrl.unsecured_allowed != DOT1X_UNSECURED_ALLOWED_MKA_FAIL))
    {
      if (intf->supp.authenticated)
        {
          dot1x_destroy_pacp(pacp);
          intf->pacp_changed = 1;
          return;
        }
      set_eap_fail(pacp, 1);
    }

 begin:
  if (pacp->state == DOT1X_PACP_BEGIN ||

      (pacp->state != DOT1X_PACP_INITIALIZE &&
       (!intf->port_enabled ||
        (pacp->enabled && !intf->ctrl.authenticator_enable))) ||

      (pacp->state == DOT1X_PACP_INITIALIZE &&
       intf->port_enabled /* extra */ &&
       (pacp->rx_eapol_logoff || pacp->rx_eapol_start)) ||

      ((pacp->state == DOT1X_PACP_AUTHENTICATING || /* extra */
        pacp->state == DOT1X_PACP_AUTHENTICATED) &&
       (!netw || (intf->snetwork && netw != intf->snetwork))))
    {
      set_state(pacp, DOT1X_PACP_INITIALIZE);
      set_enabled(pacp, 0);
      set_authenticated(pacp, 0);
      set_failed(pacp, 0);
      eap_stop(pacp);
      set_retry_count(pacp, 0);
      set_rx_eapol_logoff(pacp, 0);
      set_rx_eapol_start(pacp, 0);
      intf->pacp_changed = 1;
      goto begin; /* re-run for potential 2nd transition to UNAUTHENTICATED */
    }
  else
  if ((pacp->state == DOT1X_PACP_AUTHENTICATING &&
       pacp->eap_fail &&
       (intf->stat.auth_auth_fail_while_authenticating++ || 1)))
    {
    held:
      set_state(pacp, DOT1X_PACP_HELD);
      set_failed(pacp, 1);
      set_authenticated(pacp, 0);
      set_quiet_while(pacp, intf->ctrl.quiet_period);
      intf->pacp_changed = 1;
    }
  else
  if ((pacp->state == DOT1X_PACP_INITIALIZE &&
       intf->port_enabled /* extra */ &&
       intf->ctrl.authenticator_enable) ||

      (pacp->state == DOT1X_PACP_HELD &&
       pacp->quiet_while_ticks == 0) ||

      (pacp->state == DOT1X_PACP_UNAUTHENTICATED &&
       (pacp->rx_eapol_logoff || pacp->rx_eapol_start)) ||

      (pacp->state == DOT1X_PACP_AUTHENTICATING &&
       ((pacp->retry_count >= intf->ctrl.retry_max &&
         ((pacp->rx_eapol_start &&
           (intf->stat.auth_auth_eap_starts_while_authenticating++ || 1)) ||
          (pacp->eap_timeout &&
           (intf->stat.auth_auth_timeouts_while_authenticating++|| 1)))) ||
        (pacp->rx_eapol_logoff &&
         (intf->stat.auth_auth_eap_logoff_while_authenticating++ || 1)))) ||

      (pacp->state == DOT1X_PACP_AUTHENTICATED &&
       ((pacp->rx_eapol_logoff &&
         (intf->stat.auth_auth_eap_logoff_while_authenticated++ || 1)) ||
        !pacp->port->auth_authenticate)))
    {
      set_state(pacp, DOT1X_PACP_UNAUTHENTICATED);
      set_enabled(pacp, 1);
      set_authenticated(pacp, 0);
      set_failed(
        pacp, pacp->failed || pacp->retry_count >= intf->ctrl.retry_max);
      set_rx_eapol_logoff(pacp, 0);
      set_rx_eapol_start(pacp, 0);
      eap_stop(pacp);
      set_retry_count(pacp, 0);
      intf->pacp_changed = 1;
    }
  else
  if ((pacp->state == DOT1X_PACP_UNAUTHENTICATED &&
       pacp->port->auth_authenticate &&
       !pacp->failed &&
       /* !auth.eapStop && */
       (intf->stat.auth_enters_authenticating++ || 1)) ||

      (pacp->state == DOT1X_PACP_AUTHENTICATING &&
       pacp->retry_count < intf->ctrl.retry_max &&
       ((pacp->rx_eapol_start &&
         (intf->stat.auth_auth_eap_starts_while_authenticating++ || 1)) ||
        (pacp->eap_timeout &&
         (intf->stat.auth_auth_timeouts_while_authenticating++ || 1)))) ||

      (pacp->state == DOT1X_PACP_AUTHENTICATED &&
       ((pacp->rx_eapol_start &&
         (intf->stat.auth_auth_eap_starts_while_authenticated++ || 1)) ||
        (pacp->reauth_when_ticks == 0 &&
         intf->ctrl.reauth_enabled &&
         (intf->stat.auth_auth_reauths_while_authenticated++ || 1)) ||
        pacp->eap_fail /* extra */)))
    {
      set_state(pacp, DOT1X_PACP_AUTHENTICATING);
      set_eap_success(pacp, 0);
      set_eap_timeout(pacp, 0);
      set_eap_fail(pacp, 0);
      if (!eap_start(pacp))
        set_eap_fail(pacp, 1);
      set_failed(pacp, 0);
      set_rx_eapol_start(pacp, 0);
      set_retry_count(pacp, pacp->retry_count + 1);
      intf->pacp_changed = 1;
    }
  else
  if ((pacp->state == DOT1X_PACP_AUTHENTICATING &&
       pacp->eap_success &&
       (intf->stat.auth_auth_successes_while_authenticating++ || 1)))
    {
      set_state(pacp, DOT1X_PACP_AUTHENTICATED);
      set_authenticated(pacp, 1);
      get_results(pacp);
      /* on failed CAK creation shortcut to HELD */
      if (!pacp->cak &&
          netw->ctrl.unsecured_allowed != DOT1X_UNSECURED_ALLOWED_IMMEDIATE &&
          netw->ctrl.unsecured_allowed != DOT1X_UNSECURED_ALLOWED_MKA_FAIL)
        goto held;
      set_retry_count(pacp, 0);
      set_reauth_when(pacp, intf->ctrl.reauth_period);
      intf->pacp_changed = 1;
    }

  /* remove inactive or failed PACP instance */
  if (pacp->state == DOT1X_PACP_INITIALIZE ||

      (pacp->state == DOT1X_PACP_UNAUTHENTICATED &&
       (!pacp->port->auth_authenticate || pacp->failed)))
    {
      dot1x_destroy_pacp(pacp);
      intf->pacp_changed = 1;
    }
}

void
tick_instance_supp(Dot1xPacp *pacp)
{
  if (pacp->held_while_ticks && --pacp->held_while_ticks == 0)
    dot1x_debug_pacp(pacp, "heldWhile = 0");
}

static void
tick_instance_auth(Dot1xPacp *pacp)
{
  Dot1xIntf *intf = pacp->side->intf;
  unsigned ticks;

  if (pacp->quiet_while_ticks && --pacp->quiet_while_ticks == 0)
    dot1x_debug_pacp(pacp, "quietWhile = 0");

  if (pacp->reauth_when_ticks &&
      !pacp->port->suspended &&
      --pacp->reauth_when_ticks == 0)
    dot1x_debug_pacp(pacp, "reauthWhen = 0");

  ticks = intf->ctrl.reauth_period * DOT1X_TICKS_PER_SECOND;
  if (pacp->reauth_when_ticks > ticks)
    {
      pacp->reauth_when_ticks = ticks;
      dot1x_debug_pacp(
        pacp, "reauthWhen = %u",
        (unsigned)pacp->reauth_when_ticks / DOT1X_TICKS_PER_SECOND);
    }
}

static void
receive_msg_supp(
  Dot1xIntf *intf, const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *msg_buf, unsigned msg_len)
{
  Dot1xPacp *pacp;

  (void)dst;

  if (!intf->ctrl.supplicant_enable)
    return;

  /* find a supplicant with matching partner MAC address, take the
     supplicant without partner, i.e. the default supplicant, or start
     a new supplicant */
  if ((pacp = dot1x_pacps_lookup(&intf->supp.pacps, src)))
    ;
  else if ((pacp = rebind(&intf->supp, &zero, src)))
    ;
  else if ((pacp = dot1x_create_pacp(&intf->supp, src)))
    ;
  else
    return;

  /* pass message to higher layer if a conversation exists */
  if (pacp->eap_conversation)
    {
      eap_consume_packet(pacp->eap_conversation, msg_buf, msg_len);
      return;
    }

  /* otherwise, discard if the PACP is not currently authenticated */
  if (pacp->state != DOT1X_PACP_AUTHENTICATED)
    {
      dot1x_debug_discard(intf, "unexpected EAP packet");
      return;
    }

  /* otherwise, do a transition back to AUTHENTICATING */
  set_state(pacp, DOT1X_PACP_AUTHENTICATING);
  set_eap_success(pacp, 0);
  set_eap_timeout(pacp, 0);
  set_eap_fail(pacp, 0);
  if (!eap_start(pacp))
    set_eap_fail(pacp, 1);
  else
    eap_consume_packet(pacp->eap_conversation, msg_buf, msg_len);
  set_retry_count(pacp, pacp->retry_count + 1);
}

static void
receive_msg_auth(
  Dot1xIntf *intf, const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *msg_buf, unsigned msg_len)
{
  Dot1xPacp *pacp;
  Dot1xFormatBuffer fb;

  (void)dst;

  if (!intf->ctrl.authenticator_enable)
    return;

  /* find an authenticator with matching partner MAC address or take
     the authenticator without partner, i.e. the default authenticator */
  if ((pacp = dot1x_pacps_lookup(&intf->auth.pacps, src)))
    ;
  else if ((pacp = rebind(&intf->auth, &zero, src)))
    ;
  else
    {
      dot1x_debug_discard(
        intf, "cannot match EAP packet from %s with a conversation",
        dot1x_format_address(&fb, src));
      return;
    }

  /* ensure that higher layer conversation exists */
  if (!pacp->eap_conversation)
    {
      dot1x_debug_discard(
        intf, "unexpected EAP packet from %s",
        dot1x_format_address(&fb, src));
      return;
    }

  eap_consume_packet(pacp->eap_conversation, msg_buf, msg_len);
}

static void
send_start(Dot1xPacp *pacp)
{
  Dot1xIntf *intf = pacp->side->intf;
  const MacsecAddress *src, *dst;
  unsigned char *msdu_buf = intf->msdu;
  unsigned msdu_max = sizeof intf->msdu, msdu_len;
  unsigned char *body_buf = msdu_buf + DOT1X_EAPOL_HEADER_SIZE;
  unsigned body_max = msdu_max - DOT1X_EAPOL_HEADER_SIZE, body_len;
  Dot1xAnpduReq *req = &intf->anpdureq;
  Dot1xFormatBuffer fb;

  dot1x_debug_pacp_eapol(pacp, "txEapolStart()");

  /* embed announcement request if necessary */
  body_len = 0;
  if (intf->ctrl.listener_enable)
    {
      /* add announcement request and if it does not fit then omit */
      dot1x_generate_announcement_req(intf, req);
      if (!dot1x_encode_announcement_req(
            intf, req, body_buf + 1, body_max - 1, &body_len))
        body_len = 0;

      /* set LSB of first byte to solicit an announcement */
      body_buf[0] = 1;
      body_len++;
    }

  dot1x_encode_eapol_header(msdu_buf, 3, DOT1X_EAPOL_START, body_len);
  msdu_len = DOT1X_EAPOL_HEADER_SIZE + body_len;

  dst = dot1x_eapol_address(intf->ctrl.eapol_group);
  src = &intf->local_address;

  dot1x_debug_eap(
    intf, "sending EAP start to %s", dot1x_format_address(&fb, dst));

  if (body_len > 0)
    {
      if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_EAP) &&
          dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
        {
          dot1x_printf(intf, "    Request = %u\n", body_buf[0] & 1);
          dot1x_print_announcement_req(intf, req);
        }
    }

  dot1x_debug_eapol(
    intf, "sending EAPOL-Start to %s", dot1x_format_address(&fb, dst));

  intf->stat.eapol_start_frames_tx++;

  if (!macsec_send_uncontrolled(
        intf->macsec_intf, dst, src, msdu_buf, msdu_len))
    dot1x_error(intf, "cannot send to uncontrolled port");
}

static void
send_logoff(Dot1xPacp *pacp)
{
  Dot1xIntf *intf = pacp->side->intf;
  const MacsecAddress *src, *dst;
  Dot1xFormatBuffer fb;
  unsigned char *msdu_buf = intf->msdu;
  unsigned msdu_len = DOT1X_EAPOL_HEADER_SIZE;

  if (!intf->ctrl.logoff_enable)
    return;

  dot1x_debug_pacp_eapol(pacp, "txEapolLogoff()");

  dot1x_encode_eapol_header(msdu_buf, 3, DOT1X_EAPOL_LOGOFF, 0);

  dst = &pacp->partner;
  src = &intf->local_address;

  dot1x_debug_eap(
    intf, "sending EAP logoff to %s", dot1x_format_address(&fb, dst));

  dot1x_debug_eapol(
    intf, "sending EAPOL-Logoff to %s", dot1x_format_address(&fb, dst));

  intf->stat.eapol_logoff_frames_tx++;

  if (!macsec_send_uncontrolled(
        intf->macsec_intf, dst, src, msdu_buf, msdu_len))
    dot1x_error(intf, "cannot send to uncontrolled port");
}

static void
set_state(Dot1xPacp *pacp, int value)
{
  pacp->state = value;

  dot1x_debug_pacp(pacp, "state = %s", dot1x_pacp_state(pacp->state));
}

static void
set_enabled(Dot1xPacp *pacp, int value)
{
  Dot1xIntf *intf = pacp->side->intf;
  unsigned side_enabled;

  if (pacp->enabled == value)
    return;

  pacp->enabled = value;

  dot1x_debug_pacp(pacp, "enabled = %s", dot1x_bool(pacp->enabled));

  if (!value)
    pacp->side->enabled_count--;
  else
    pacp->side->enabled_count++;

  side_enabled = pacp->side->enabled_count > 0;

  if (pacp->side->enabled == side_enabled)
    return;

  pacp->side->enabled = side_enabled;

  dot1x_debug_logon_pacp(
    intf, "%s.enabled = %s", pacp->side == &intf->supp ? "supp" : "auth",
    dot1x_bool(pacp->side->enabled));
}

static void
set_authenticated(Dot1xPacp *pacp, int value)
{
  Dot1xIntf *intf = pacp->side->intf;
  unsigned side_authenticated;

  if (pacp->authenticated == value)
    return;

  pacp->authenticated = value;

  dot1x_debug_pacp(
    pacp, "authenticated = %s", dot1x_bool(pacp->authenticated));

  if (!value)
    pacp->side->authenticated_count--;
  else
    pacp->side->authenticated_count++;

  /* update interface-wide supp/auth authenticated status */
  /* always zero if virtual ports in use */
  if (!intf->ctrl.virtual_ports_enable)
    side_authenticated = pacp->side->authenticated_count > 0;
  else
    side_authenticated = 0;

  if (pacp->side->authenticated == side_authenticated)
    return;

  pacp->side->authenticated = side_authenticated;

  if (pacp->side->authenticated && pacp->authenticated)
    pacp->side->last_authenticated_pacp = pacp;
  else
    pacp->side->last_authenticated_pacp = NULL;

  dot1x_debug_logon_pacp(
    intf, "%s.authenticated = %s", pacp->side == &intf->supp ? "supp" : "auth",
    dot1x_bool(pacp->side->authenticated));
}

static void
get_results(Dot1xPacp *pacp)
{
  MacsecSessionId session_id;
  MacsecMsk msk;
  MacsecAuthData auth_data;

  eap_get_results(pacp->eap_conversation, &session_id, &msk, &auth_data);
  eap_destroy(pacp->eap_conversation);
  pacp->eap_conversation = NULL;
  dot1x_process_eap_results(pacp, &session_id, &msk, &auth_data);
  /* clear MSK in stack */
  memset(&msk, 0, sizeof msk);
}

static void
set_failed(Dot1xPacp *pacp, int value)
{
  Dot1xIntf *intf = pacp->side->intf;
  unsigned side_failed;

  if (pacp->failed == value)
    return;

  pacp->failed = value;

  dot1x_debug_pacp(pacp, "failed = %s", dot1x_bool(pacp->failed));

  /* set interface-wide failed status if not already set (cleared by Logon) */
  /* always zero if virtual ports in use */
  if (!intf->ctrl.virtual_ports_enable)
    side_failed = pacp->side->failed || value;
  else
    side_failed = 0;

  if (side_failed == pacp->side->failed)
    return;

  pacp->side->failed = side_failed;
  dot1x_debug_logon_pacp(
    intf, "%s.failed = %s", pacp->side == &intf->supp ? "supp" : "auth",
    dot1x_bool(pacp->side->failed));

  if (side_failed)
    dot1x_error(
      intf, "EAP %s failed",
      pacp->side == &intf->supp ? "supplicant" : "authenticator");

  if (pacp->failed)
    dot1x_process_eap_failure(pacp);
}

static void
eap_stop(Dot1xPacp *pacp)
{
  if (!pacp->eap_conversation)
    return;

  dot1x_debug_pacp_eap(pacp, "eapStop()");
  eap_destroy(pacp->eap_conversation);
  pacp->eap_conversation = NULL;
}

static unsigned
eap_start(Dot1xPacp *pacp)
{
  Dot1xIntf *intf = pacp->side->intf;
  Dot1xPort *port = pacp->port;
  Dot1xNetwork *netw;
  EapPartner ep;

  /* kill any existing conversation */
  if (pacp->eap_conversation)
    eap_stop(pacp);

  /* clear network */
  if (pacp->network)
    dot1x_unlink_pacp_network(pacp);

  /* get a network for authentication */
  netw = dot1x_choose_network(port);
  dot1x_link_pacp_network(pacp, netw);

  /* set up EAP authentication parameters */
  memset(&ep, 0, sizeof ep);
  memcpy(&ep.mac_address, &pacp->partner, sizeof ep.mac_address);
  dot1x_nidcpy(&ep.nid, &netw->nid);

  /* get a new conversation */
  dot1x_debug_pacp_eap(pacp, "eapStart()");
  if (pacp->side == &intf->supp)
    pacp->eap_conversation = eap_create_supplicant(intf->eap_context, &ep);
  else
    pacp->eap_conversation = eap_create_authenticator(intf->eap_context, &ep);

  if (!pacp->eap_conversation)
    {
      dot1x_error(intf, "cannot start EAP conversation");
      dot1x_unlink_pacp_network(pacp);
      return 0;
    }

  return 1;
}

static void
set_eap_success(Dot1xPacp *pacp, int value)
{
  if (pacp->eap_success == value)
    return;

  pacp->eap_success = value;

  dot1x_debug_pacp_eap(pacp, "eapSuccess = %s", dot1x_bool(pacp->eap_success));
}

static void
set_eap_timeout(Dot1xPacp *pacp, int value)
{
  if (pacp->eap_timeout == value)
    return;

  pacp->eap_timeout = value;

  dot1x_debug_pacp_eap(pacp, "eapTimeout = %s", dot1x_bool(pacp->eap_timeout));
}

static void
set_eap_fail(Dot1xPacp *pacp, int value)
{
  if (pacp->eap_fail == value)
    return;

  pacp->eap_fail = value;

  dot1x_debug_pacp_eap(pacp, "eapFail = %s", dot1x_bool(pacp->eap_fail));
}

static void
set_rx_eapol_start(Dot1xPacp *pacp, int value)
{
  if (pacp->rx_eapol_start == value)
    return;

  pacp->rx_eapol_start = value;

  dot1x_debug_pacp_eapol(
    pacp, "rxEapolStart = %s", dot1x_bool(pacp->rx_eapol_start));
}

static void
set_rx_eapol_logoff(Dot1xPacp *pacp, int value)
{
  if (pacp->rx_eapol_logoff == value)
    return;

  pacp->rx_eapol_logoff = value;

  dot1x_debug_pacp_eapol(
    pacp, "rxEapolLogoff = %s", dot1x_bool(pacp->rx_eapol_logoff));
}

static void
set_retry_count(Dot1xPacp *pacp, unsigned value)
{
  if (pacp->retry_count == value)
    return;

  pacp->retry_count = value;

  dot1x_debug_pacp(pacp, "retryCount = %u", (unsigned)pacp->retry_count);
}

static void
set_held_while(Dot1xPacp *pacp, unsigned value)
{
  if (pacp->held_while_ticks == value * DOT1X_TICKS_PER_SECOND)
    return;

  pacp->held_while_ticks = value * DOT1X_TICKS_PER_SECOND;

  dot1x_debug_pacp(
    pacp, "heldWhile = %u",
    (unsigned)pacp->held_while_ticks / DOT1X_TICKS_PER_SECOND);
}

static void
set_quiet_while(Dot1xPacp *pacp, unsigned value)
{
  if (pacp->quiet_while_ticks == value * DOT1X_TICKS_PER_SECOND)
    return;

  pacp->quiet_while_ticks = value * DOT1X_TICKS_PER_SECOND;

  dot1x_debug_pacp(
    pacp, "quietWhile = %u",
    (unsigned)pacp->quiet_while_ticks / DOT1X_TICKS_PER_SECOND);
}

static void
set_reauth_when(Dot1xPacp *pacp, unsigned value)
{
  if (pacp->reauth_when_ticks == value * DOT1X_TICKS_PER_SECOND)
    return;

  pacp->reauth_when_ticks = value * DOT1X_TICKS_PER_SECOND;

  dot1x_debug_pacp(
    pacp, "reauthWhen = %u",
    (unsigned)pacp->reauth_when_ticks / DOT1X_TICKS_PER_SECOND);
}

static Dot1xPacp *
rebind(
  Dot1xPacpSide *side, const MacsecAddress *oldpart,
  const MacsecAddress *newpart)
{
  Dot1xPacp *old, *new;
  Dot1xIntf *intf = side->intf;
  Dot1xPort *newport;
  Dot1xCak *cak;
  Dot1xNetwork *netw;
  Dot1xFormatBuffer fb;

  /* ensure there isn't existing entry for the new address */
  if (dot1x_pacps_lookup(&side->pacps, newpart))
    return NULL;

  /* look up and remove entry for the old address, if any */
  if (!(old = dot1x_pacps_remove(&side->pacps, oldpart)))
    return NULL;

  /* get port reference with the new address */
  if (!memcmp(newpart, &zero, sizeof *newpart))
    newport = dot1x_refer_port(intf, NULL);
  else
    newport = dot1x_refer_port(intf, newpart);
  /* if failed put back the old PACP entry and fail */
  if (!newport)
    {
      dot1x_pacps_insert(&side->pacps, &old->partner);
      return NULL;
    }

  /* return old port reference (possibly to the same port as the new
     one) */
  dot1x_unref_port(old->port);
  old->port = NULL;

  /* get PACP entry with the new address (might be the same object as
     the old one was) */
  new = dot1x_pacps_insert(&side->pacps, newpart);

  /* move contents if different objects */
  if (new != old)
    {
      if ((cak = old->cak))
        dot1x_unlink_pacp_cak(old);
      if ((netw = old->network))
        dot1x_unlink_pacp_network(old);

      *new = *old;
      memset(old, 0, sizeof *old);

      if (netw)
        dot1x_link_pacp_network(new, netw);
      if (cak)
        dot1x_link_pacp_cak(new, cak);
    }

  /* put new port reference into place */
  new->port = newport;

  dot1x_debug_pacp(
    new, "associated with partner %s",
    dot1x_format_address(&fb, &new->partner));

  return new;
}
