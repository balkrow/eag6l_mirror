/**

   IEEE 802.1X-2010 network annoucement support.

   File: dot1xpae_netann.c

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#include "dot1xpae_netann.h"
#include "dot1xpae_logon.h"
#include "dot1xpae_pacp.h"
#include "dot1xpae_mka.h"
#include "dot1xpae_anpdu.h"
#include "dot1xpae_eapol.h"
#include "dot1xpae_util.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

/*
 * Announcement time values in seconds from 802.1X.
 */
#define PERIOD 5

/*
 * Time in seconds to remember an announcement request.
 */
#define REQ_HOLD 2

/*
 * Announcement rate limit in number of packets per second.
 */
#define RATE_LIMIT 5

/*
 * Run per-network announcement functions.
 */
static void
step_network(Dot1xNetwork *n);

/*
 * Run announcement functions of a real or virtual port.
 */
static void
step_port(Dot1xPort *port);

/*
 * Advance per-network timers.
 */
static void
tick_network(Dot1xNetwork *n);

/*
 * Expire non-configured (finite lifetime) network.
 */
static void
expire_network(Dot1xNetwork *n);

/*
 * Advance announcement timers of a real or virtual port.
 */
static void
tick_port(Dot1xPort *port);

/*
 * Send a generic or specific EAPOL-Announcement packet depending on
 * whether the port is the real port or a virtual port.
 */
static void
send_announcement(Dot1xPort *port);

/*
 * Send an EAPOL-Announcement-Req packet.
 */
static void
send_announcement_req(Dot1xIntf *intf);

/*
 * Fill network data to be sent in an announcement packet.
 */
static void
send_network(
  Dot1xPort *port, Dot1xAnpduNetwork *an, const Dot1xNetwork *n);

/*
 * Fill network data to be sent in an announcement request packet.
 */
static void
send_network_req(
  Dot1xIntf *intf, Dot1xAnpduReqNetwork *rn, const Dot1xNetwork *n);

/*
 * Handle network data received in an announcement packet of the
 * specific type (generic or specific) and update the given network
 * object, provided the received data is valid.
 */
static void
receive_network(unsigned type, Dot1xNetwork *n, const Dot1xAnpduNetwork *an);

/*
 * Handle network data received in an announcement request packet.
 */
static void
receive_network_req(Dot1xNetwork *n, const Dot1xAnpduReqNetwork *rn);

/*
 * Return 1 if network policy controls and received network
 * information allow the network to be used for setting up
 * connectivity, 0 otherwise.
 */
static int
usable_network(Dot1xNetwork *n);

/*
 * Fill an access capabilities structure from a network object for
 * sending in an announcement or announcement request.
 */
static void
send_access_capabilities(Dot1xAccessCapabilities *ac, const Dot1xNetwork *n);

/*
 * Link port with a requested network and vice versa.
 */
static void
link_port_rnetwork(Dot1xPort *port, Dot1xNetwork *n);

/*
 * Break link between a port and the requested network it was linked
 * with.
 */
static void
unlink_port_rnetwork(Dot1xPort *port);

/*
 * Public functions.
 */

Dot1xNetwork *
dot1x_insert_network(Dot1xIntf *intf, const Dot1xNid *nid)
{
  Dot1xNetwork *n;
  Dot1xFormatBuffer fb;

  if (!(n = dot1x_networks_insert(&intf->networks, nid)))
    {
      dot1x_error(intf, "out of network objects");
      return NULL;
    }

  n->intf = intf;
  n->ctrl = dot1x_default_network_ctrl;

  macsec_list_init(&n->ports);
  macsec_list_init(&n->pacps);
  macsec_list_init(&n->caks);

  dot1x_debug_network(
    n, "created with NID %s", dot1x_format_nid(&fb, &n->nid));

  if (!intf->snetwork && !dot1x_nidcmp(&intf->ctrl.selected_nid, nid))
    intf->snetwork = n;

  return n;
}

void
dot1x_remove_network(Dot1xNetwork *n)
{
  Dot1xIntf *intf = n->intf;

  dot1x_debug_network(n, "being destroyed");

  if (intf->snetwork && intf->snetwork == n)
    intf->snetwork = NULL;

  macsec_list_foreach(
    &n->caks, offsetof(Dot1xCak, network_link),
    dot1x_unlink_cak_network);

  macsec_list_foreach(
    &n->pacps, offsetof(Dot1xPacp, network_link),
    dot1x_unlink_pacp_network);

  macsec_list_foreach(
    &n->ports, offsetof(Dot1xPort, rnetwork_link),
    unlink_port_rnetwork);

  assert(dot1x_networks_remove(&intf->networks, &n->nid) == n);
  memset(n, 0, sizeof *n);
}

int
dot1x_step_netann(Dot1xIntf *intf)
{
  /* update per-interface announcer state */

  if (intf->ctrl.announcer_enable &&
      intf->port_enabled)
    {
      /* send initial generic announcement when portEnabled and
         announcer.enable become true */

      if (!intf->initial_announcement_sent)
        {
          intf->initial_announcement_sent = 1;
          intf->rport.announcement_solicited = 0;
          intf->announcement_ticks =
            PERIOD * DOT1X_TICKS_PER_SECOND;
          intf->announcement_credit = RATE_LIMIT;
          send_announcement(&intf->rport);
        }
      else

      /* send periodic generic announcement */
      /* rate limit shared by all announcements on all ports */

      if (!intf->announcement_ticks && intf->announcement_credit > 0)
        {
          intf->announcement_ticks =
            PERIOD * DOT1X_TICKS_PER_SECOND;
          intf->announcement_credit--;
          send_announcement(&intf->rport);
        }
    }
  else
    {
      /* reset flag when portEnabled or announcer.enable becomes
         false */

      if (intf->initial_announcement_sent)
        intf->initial_announcement_sent = 0;
    }

  /* update per-interface listener state */

  if (intf->ctrl.listener_enable &&
      !intf->ctrl.supplicant_enable &&
      intf->port_enabled)
    {
      /* send announcement request when portEnabled and
         listener.enable become true */

      if (!intf->announcement_req_sent)
        {
          intf->announcement_req_sent = 1;
          send_announcement_req(intf);
        }
    }
  else
    {
      /* reset flags when portEnabled or listener.enable becomes
         false */

      if (intf->announcement_req_sent)
        intf->announcement_req_sent = 0;
    }

  /* update per-network state */
  dot1x_networks_foreach(&intf->networks, step_network);

  /* update per-port state */
  dot1x_foreach_port(intf, step_port);

  return 0;
}

void
dot1x_tick_netann(Dot1xIntf *intf)
{
  if (intf->announcement_ticks && !--intf->announcement_ticks)
    intf->changed = 1;

  if (intf->announcement_credit < RATE_LIMIT)
    intf->announcement_credit +=
      RATE_LIMIT / DOT1X_TICKS_PER_SECOND;

  dot1x_foreach_port(intf, tick_port);
  dot1x_networks_foreach(&intf->networks, tick_network);
}

void
dot1x_receive_announcement(
  Dot1xIntf *intf,
  const MacsecAddress *dst,
  unsigned type,
  const unsigned char *buf,
  unsigned len)
{
  Dot1xAnpdu *ann = &intf->anpdu;

  intf->stat.eapol_announcements_rx++;

  if (!intf->ctrl.listener_enable)
    return;

  if (type == DOT1X_EAPOL_ANNOUNCEMENT_SPC && (dst->buf[0] & 1))
    {
      dot1x_debug_discard(intf, "specific announcement to multicast address");
      return;
    }

  if (!dot1x_decode_announcement(intf, ann, buf, len))
    {
      dot1x_debug_discard(intf, "cannot decode announcement");
      return;
    }

  dot1x_debug_announcement(
    intf, "received %s announcement with %u named networks",
    type == DOT1X_EAPOL_ANNOUNCEMENT_SPC ? "specific" : "generic",
    ann->network_num);

  if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_ANNOUNCEMENT) &&
      dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
    dot1x_print_announcement(intf, ann);

  dot1x_process_announcement(intf, type, ann);
}

void
dot1x_receive_announcement_req(
  Dot1xIntf *intf,
  const MacsecAddress *src,
  const unsigned char *buf,
  unsigned len)
{
  Dot1xAnpduReq *req = &intf->anpdureq;
  Dot1xPort *port = NULL;

  intf->stat.eapol_announcement_reqs_rx++;

  if (!intf->ctrl.announcer_enable)
    return;

  /* get temporary port reference */
  if (!(port = dot1x_refer_port(intf, src)))
    goto end;

  /* it's a solicit if there is no body or LSB of first byte is set */
  if (len <= 0 || (buf[0] & 1))
    {
      port->announcement_solicited = 1;
      intf->changed = 1;
    }

  if (len <= 0)
    goto end;

  if (!dot1x_decode_announcement_req(intf, req, buf + 1, len - 1))
    {
      dot1x_debug_discard(intf, "cannot decode announcement request");
      goto end;
    }

  dot1x_debug_announcement(
    intf, "received announcement request with %u networks", req->network_num);

  if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_ANNOUNCEMENT) &&
      dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
    {
      dot1x_printf(intf, "    Request = %u\n", buf[0] & 1);
      dot1x_print_announcement_req(intf, req);
    }

  dot1x_process_announcement_req(port, req);

 end:
  /* return temporary port reference */
  if (port)
    dot1x_unref_port(port);
}

void
dot1x_generate_announcement(Dot1xPort *port, Dot1xAnpdu *ann)
{
  Dot1xIntf *intf = port->intf;
  Dot1xAnpduNetwork *an;
  Dot1xNetwork *n;

  memset(ann, 0, sizeof *ann);

  /* handle default network */
  send_network(port, &ann->default_network, &intf->dnetwork);

  /* handle named networks */
  n = NULL;
  while ((n = dot1x_networks_next(&intf->networks, n)))
    {
      if (ann->network_num >= DOT1X_MAX_NETWORKS)
        {
          dot1x_error(intf, "too many networks to announce, omitting some");
          continue;
        }
      an = &ann->network_tab[ann->network_num++];
      send_network(port, an, n);
    }
}

void
dot1x_process_announcement(
  Dot1xIntf *intf, unsigned eapol_type, const Dot1xAnpdu *ann)
{
  const Dot1xAnpduNetwork *an;
  Dot1xNetwork *n;
  int i;

  /* handle default network */
  an = &ann->default_network;
  n = &intf->dnetwork;
  receive_network(eapol_type, n, an);

  /* expire previously received networks */
  dot1x_networks_foreach(&intf->networks, expire_network);

  /* handle named networks */
  for (i = 0; i < ann->network_num; i++)
    {
      an = &ann->network_tab[i];
      if (an->nid.len <= 0)
        {
          dot1x_debug_discard(intf, "named network with empty NID");
          continue;
        }

      /* lookup configured (infinite life) or non-configured network
         or create a new non-configured network */
      if ((n = dot1x_networks_lookup(&intf->networks, &an->nid)))
        ;
      else if ((n = dot1x_insert_network(intf, &an->nid)))
        n->life_ticks = 1;
      else
        continue;

      /* keep non-configured (finite life) networks for twice the
         transmission period */
      if (n->life_ticks)
        n->life_ticks = 2 * PERIOD * DOT1X_TICKS_PER_SECOND;

      receive_network(eapol_type, n, an);

      dot1x_refresh_caks(intf, &n->nid, &n->stat.kmd);
    }
}

void
dot1x_generate_announcement_req(Dot1xIntf *intf, Dot1xAnpduReq *req)
{
  Dot1xAnpduReqNetwork *rn;
  Dot1xNetwork *n;

  memset(req, 0, sizeof *req);

  /* send selected network first, if configured */
  if (intf->snetwork)
    {
      rn = &req->network_tab[req->network_num++];
      send_network_req(intf, rn, intf->snetwork);
    }

  /* send other configured (infinite life) networks */
  n = NULL;
  while ((n = dot1x_networks_next(&intf->networks, n)))
    {
      if (n == intf->snetwork || n->life_ticks > 0)
        continue;

      if (req->network_num >= DOT1X_MAX_NETWORKS)
        {
          dot1x_error(intf, "too many networks to request, omitting some");
          continue;
        }

      rn = &req->network_tab[req->network_num++];
      send_network_req(intf, rn, n);
    }
}

void
dot1x_process_announcement_req(Dot1xPort *port, const Dot1xAnpduReq *req)
{
  Dot1xIntf *intf = port->intf;
  const Dot1xAnpduReqNetwork *rn;
  Dot1xNetwork *n, *best_network;
  Dot1xFormatBuffer fb;
  int i;

  best_network = NULL;
  for (i = 0; i < req->network_num; i++)
    {
      rn = &req->network_tab[i];

      /* lookup a network object, ignore this NID if not found */
      /* empty NID means the default network */
      if (rn->nid.len <= 0)
        n = &intf->dnetwork;
      else if (!(n = dot1x_networks_lookup(&intf->networks, &rn->nid)))
        continue;

      receive_network_req(n, rn);

      /* remember the first usable network as the most desirable one */
      if (!best_network && usable_network(n))
        best_network = n;
    }

  /* change requested network of a port if needed */
  if (best_network != port->rnetwork)
    {
      if (best_network)
        dot1x_debug_network_port(
          port, "NID %s requested", dot1x_format_nid(&fb, &best_network->nid));
      else
        dot1x_debug_network_port(port, "no known NID requested");

      if (port->rnetwork)
        unlink_port_rnetwork(port);
      if (best_network)
        {
          link_port_rnetwork(port, best_network);
          port->rnetwork_ticks = REQ_HOLD * DOT1X_TICKS_PER_SECOND;
        }

      if (port->rnetwork)
        dot1x_refresh_caks(
          intf, &port->rnetwork->nid, &port->rnetwork->ctrl.kmd);
    }
}

/*
 * Public variables.
 */

const Dot1xNetworkCtrl dot1x_default_network_ctrl = {
  .use_eap = DOT1X_USE_EAP_IMMEDIATE,
  .unauth_allowed = DOT1X_UNAUTH_ALLOWED_NEVER,
  .unsecured_allowed = DOT1X_UNSECURED_ALLOWED_NEVER,
  .kmd = {.buf= {0}, .len = 0},
};

/*
 * Static functions.
 */

static void
step_network(Dot1xNetwork *n)
{
  /* remove non-configured (finite lifetime) network when expired
     unless there are CAKs or PACPs using it */
  if (n->life_ticks == 1 &&
      macsec_list_empty(&n->caks) &&
      macsec_list_empty(&n->pacps))
    dot1x_remove_network(n);
}

static void
step_port(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;

  if (intf->ctrl.announcer_enable &&
      intf->port_enabled)
    {
      /* send solicited announcement */
      /* rate limit shared by all announcements on all ports */

      if (port->announcement_solicited && intf->announcement_credit > 0)
        {
          port->announcement_solicited = 0;
          intf->announcement_credit--;
          send_announcement(port);
        }
    }
  else
    {
      /* reset flag when portEnabled or announcer.enable becomes
         false */

      if (port->announcement_solicited)
        port->announcement_solicited = 0;
    }

  if (port->rnetwork && port->rnetwork_ticks == 1)
    {
      unlink_port_rnetwork(port);
      port->rnetwork_ticks = 0;
    }
}

static void
tick_port(Dot1xPort *port)
{
  if (port->rnetwork_ticks > 1)
    port->rnetwork_ticks--;
}

static void
tick_network(Dot1xNetwork *n)
{
  if (n->life_ticks > 1)
    n->life_ticks--;
}

static void
expire_network(Dot1xNetwork *n)
{
  if (n->life_ticks > 1)
    n->life_ticks = 1;
}

static void
send_announcement(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;
  Dot1xPeer *peer;
  unsigned char *msdu_buf = intf->msdu;
  unsigned msdu_max = sizeof intf->msdu, msdu_len;
  unsigned char *body_buf = msdu_buf + DOT1X_EAPOL_HEADER_SIZE;
  unsigned body_max = msdu_max - DOT1X_EAPOL_HEADER_SIZE, body_len;
  const MacsecAddress *dst;
  Dot1xAnpdu *ann = &intf->anpdu;
  unsigned type;
  Dot1xFormatBuffer fb;

  if (port == &intf->rport)
    {
      type = DOT1X_EAPOL_ANNOUNCEMENT_GEN;
      dst = dot1x_eapol_address(intf->ctrl.eapol_group);
    }
  else
    {
      type = DOT1X_EAPOL_ANNOUNCEMENT_SPC;
      peer = (void *)((unsigned char *)port - offsetof(Dot1xPeer, vport));
      dst = &peer->peer_address;
    }

  dot1x_generate_announcement(port, ann);

  if (!dot1x_encode_announcement(intf, ann, body_buf, body_max, &body_len))
    return;

  dot1x_encode_eapol_header(msdu_buf, 3, type, body_len);
  msdu_len = DOT1X_EAPOL_HEADER_SIZE + body_len;

  if (type == DOT1X_EAPOL_ANNOUNCEMENT_SPC)
    dot1x_debug_announcement(
      intf, "sending specific announcement with %u named networks to %s",
      ann->network_num, dot1x_format_address(&fb, dst));
  else
    dot1x_debug_announcement(
      intf, "sending generic announcement with %u named networks",
      ann->network_num);

  if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_ANNOUNCEMENT) &&
      dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
    dot1x_print_announcement(intf, ann);

  dot1x_debug_eapol(
    intf, "sending EAPOL-%s packet to %s",
    dot1x_eapol_type(type), dot1x_format_address(&fb, dst));

  intf->stat.eapol_announcements_tx++;

  if (!macsec_send_uncontrolled(
        intf->macsec_intf, dst, &intf->local_address, msdu_buf, msdu_len))
    dot1x_error(intf, "cannot send to uncontrolled port");
}

static void
send_announcement_req(Dot1xIntf *intf)
{
  unsigned char *msdu_buf = intf->msdu;
  unsigned msdu_max = sizeof intf->msdu, msdu_len;;
  unsigned char *body_buf = msdu_buf + DOT1X_EAPOL_HEADER_SIZE;
  unsigned body_max = msdu_max - DOT1X_EAPOL_HEADER_SIZE, body_len;
  const MacsecAddress *dst, *src;
  unsigned type;
  Dot1xAnpduReq *req = &intf->anpdureq;
  Dot1xFormatBuffer fb;

  dot1x_generate_announcement_req(intf, req);

  if (!dot1x_encode_announcement_req(
        intf, req, body_buf + 1, body_max - 1, &body_len))
    return;

  /* set LSB of first byte to solicit an announcement */
  body_buf[0] = 1;
  body_len++;

  type = DOT1X_EAPOL_ANNOUNCEMENT_REQ;
  dot1x_encode_eapol_header(msdu_buf, 3, type, body_len);
  msdu_len = DOT1X_EAPOL_HEADER_SIZE + body_len;

  dst = dot1x_eapol_address(intf->ctrl.eapol_group);
  src = &intf->local_address;

  dot1x_debug_announcement(
    intf, "sending announcement request with %u networks", req->network_num);

  if (dot1x_debug_enabled(intf, DOT1X_MESSAGE_ANNOUNCEMENT) &&
      dot1x_debug_enabled(intf, DOT1X_MESSAGE_PACKET_DETAIL))
    dot1x_print_announcement_req(intf, req);

  dot1x_debug_eapol(
    intf, "sending EAPOL-%s packet to %s",
    dot1x_eapol_type(type), dot1x_format_address(&fb, dst));

  intf->stat.eapol_announcement_reqs_tx++;

  if (!macsec_send_uncontrolled(
        intf->macsec_intf, dst, src, msdu_buf, msdu_len))
    dot1x_error(intf, "cannot send to uncontrolled port");
}

static void
send_network(Dot1xPort *port, Dot1xAnpduNetwork *an, const Dot1xNetwork *n)
{
  Dot1xIntf *intf = port->intf;
  Dot1xAccessInformation *ai = &an->access_information;
  int i;

  dot1x_nidcpy(&an->nid, &n->nid);

  if (n == port->cnetwork &&
      (port->connect == DOT1X_CONNECT_AUTHENTICATED ||
       port->connect == DOT1X_CONNECT_SECURE))
    ai->access_status = DOT1X_ACCESS_STATUS_EXPECTED_ACCESS;
  else
    ai->access_status = DOT1X_ACCESS_STATUS_NO_ACCESS;

  switch (n->ctrl.unauth_allowed)
    {
    default:
    case DOT1X_UNAUTH_ALLOWED_NEVER:
      ai->unauthenticated_access = DOT1X_UNAUTHENTICATED_ACCESS_NO_ACCESS;
      break;
    case DOT1X_UNAUTH_ALLOWED_IMMEDIATE:
      ai->unauthenticated_access = DOT1X_UNAUTHENTICATED_ACCESS_OPEN_ACCESS;
      break;
    case DOT1X_UNAUTH_ALLOWED_AUTH_FAIL:
      ai->unauthenticated_access =
        DOT1X_UNAUTHENTICATED_ACCESS_FALLBACK_ACCESS;
      break;
    }

  send_access_capabilities(&ai->access_capabilities, n);

  if (n == port->rnetwork)
    ai->access_requested = 1;

  if (intf->ctrl.virtual_ports_enable)
    ai->virtual_port_access = 1;
  else if (intf->ctrl.mka_enable)
    ai->group_access = 1;

  an->has_access_information = 1;

  if (intf->ctrl.mka_enable && intf->ctrl.macsec_protect)
    {
      for (i = 0; i < MACSEC_CIPHER_SUITE_COUNT; i++)
        {
          an->cipher_suites[i].implemented = 1;
          an->cipher_suites[i].capability =
            MACSEC_CAPABILITY_CONFIDENTIALITY_WITH_OFFSET;
        }
      an->has_cipher_suites = 1;
    }

  if (intf->ctrl.mka_enable &&
      intf->ctrl.cak_caching_enable &&
      n->ctrl.kmd.len > 0)
    {
      dot1x_kmdcpy(&an->kmd, &n->ctrl.kmd);
      an->has_kmd = 1;
    }
}

static void
send_network_req(
  Dot1xIntf *intf, Dot1xAnpduReqNetwork *rn, const Dot1xNetwork *n)
{
  Dot1xAccessInformation *ai = &rn->access_information;

  (void)intf;
  memcpy(rn->nid.buf, n->nid.buf, n->nid.len);
  rn->nid.len = n->nid.len;

  memset(ai, 0, sizeof *ai);
  send_access_capabilities(&ai->access_capabilities, n);
  rn->has_access_information = 1;
}

static void
receive_network(
  unsigned type, Dot1xNetwork *n, const Dot1xAnpduNetwork *an)
{
  Dot1xIntf *intf = n->intf;
  Dot1xNetworkStat *s = &n->stat;
  Dot1xAccessInformation *ai = &s->access_information;
  const Dot1xAccessInformation *aai = &an->access_information;
  int i;

  /* 802.1X section 10 probably allows specific announcements to
     contain networks without virtual port access set but this
     implementation ignores them */

  if (type == DOT1X_EAPOL_ANNOUNCEMENT_SPC &&
      (!an->has_access_information || !aai->virtual_port_access))
    {
      dot1x_debug_discard(
        intf, "specific announcement network without virtual port access");
      return;
    }

  if (!an->has_access_information)
    s->access_information_valid = 0;
  else
    {
      /* if virtual ports are used take access status and access
         requested from specific announcements only (802.1X section
         10.1) */

      if (!aai->virtual_port_access || type == DOT1X_EAPOL_ANNOUNCEMENT_SPC)
        {
          ai->access_status = aai->access_status;
          ai->access_requested = aai->access_requested;
        }

      ai->unauthenticated_access = aai->unauthenticated_access;
      ai->access_capabilities = aai->access_capabilities;
      ai->virtual_port_access = aai->virtual_port_access;
      ai->group_access = aai->group_access;

      s->access_information_valid = 1;
    }

  if (!an->has_cipher_suites)
    s->cipher_suites_valid = 0;
  else
    {
      for (i = 0; i < MACSEC_CIPHER_SUITE_COUNT; i++)
        s->cipher_suites[i] = an->cipher_suites[i];
      s->cipher_suites_valid = 1;
    }

  if (!an->has_kmd || an->kmd.len <= 0)
    s->kmd_valid = 0;
  else
    {
      dot1x_kmdcpy(&s->kmd, &an->kmd);
      s->kmd_valid = 1;
    }
}

static void
receive_network_req(Dot1xNetwork *n, const Dot1xAnpduReqNetwork *rn)
{
  Dot1xNetworkStat *s = &n->stat;

  if (!rn->has_access_information)
    s->access_information_valid = 0;
  else
    {
      s->access_information = rn->access_information;
      s->access_information_valid = 1;
    }
}

static int
usable_network(Dot1xNetwork *n)
{
  Dot1xIntf *intf = n->intf;
  Dot1xAccessInformation *ai = &n->stat.access_information;
  Dot1xAccessCapabilities *ac = &ai->access_capabilities;

  /* missing access information is ok */
  if (!n->stat.access_information_valid)
    return 1;

  /* check EAP capability */
  if (n->ctrl.use_eap != DOT1X_USE_EAP_NEVER &&
      !ac->eap && !ac->eap_mka && !ac->eap_mka_macsec)
    goto fail;

  /* check MKA capability */
  if (intf->ctrl.mka_enable &&
      !ac->eap_mka && !ac->eap_mka_macsec && !ac->mka && !ac->mka_macsec)
    goto fail;

  /* check MACsec capability */
  if (intf->ctrl.macsec_protect &&
      !ac->eap_mka_macsec && !ac->mka_macsec)
    goto fail;

  return 1;

 fail:
  dot1x_debug_network(n, "requested without sufficient capabilities");
  return 0;
}

static void
send_access_capabilities(Dot1xAccessCapabilities *ac, const Dot1xNetwork *n)
{
  const Dot1xIntf *intf = n->intf;

  memset(ac, 0xff, sizeof *ac);

  if ((!intf->ctrl.authenticator_enable && !intf->ctrl.supplicant_enable) ||
      n->ctrl.use_eap == DOT1X_USE_EAP_NEVER)
    {
      ac->eap = 0;
      ac->eap_mka = 0;
      ac->eap_mka_macsec = 0;
    }
  if (!intf->ctrl.mka_enable)
    {
      ac->eap_mka = 0;
      ac->eap_mka_macsec = 0;
      ac->mka = 0;
      ac->mka_macsec = 0;
    }
  if (!intf->ctrl.macsec_protect)
    {
      ac->eap_mka_macsec = 0;
      ac->mka_macsec = 0;
    }
  ac->higher_layer = 0;
  ac->higher_layer_fallback = 0;
  ac->vendor_specific = 0;
}

static void
link_port_rnetwork(Dot1xPort *port, Dot1xNetwork *n)
{
  port->rnetwork = n;
  macsec_list_append(&n->ports, &port->rnetwork_link);
  dot1x_reref_port(port);
}

static void
unlink_port_rnetwork(Dot1xPort *port)
{
  macsec_list_remove(&port->rnetwork_link);
  port->rnetwork = NULL;
  dot1x_unref_port(port);
}
