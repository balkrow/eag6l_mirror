/**

   IEEE 802.1X-2010 Logon process.

   File: dot1xpae_logon.c

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#include "dot1xpae_logon.h"
#include "dot1xpae_pacp.h"
#include "dot1xpae_mka.h"
#include "dot1xpae_anpdu.h"
#include "dot1xpae_eapol.h"
#include "dot1xpae_util.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>

typedef struct {
  Dot1xIntf *intf;
  const MacsecAddress *partner;
  const MacsecSessionId *session_id;
} KeyDerivationArg;

/*
 * Update CAK timers.
 */
static void
tick_cak(Dot1xCak *cak);

/*
 * Update the state of a CAK.
 */
static void
step_cak(Dot1xCak *cak);

/*
 * Update the states of the per-port `supp.authenticate' and
 * `auth.authenticate' variables. Return nonzero if they were changed.
 */
static void
step_authenticate(Dot1xPort *port);

/*
 * Update the state of the `connect' variable and the connected
 * network.
 */
static void
step_connect(Dot1xPort *port);

/*
 * Find an enabled CAK different from the one given but with the same
 * NID. If the given CAK is a pairwise CAK, consider pairwise and
 * group CAKs. If the given CAK is a group CAK, consider only group
 * CAKs.
 */
static Dot1xCak *
find_better_cak(Dot1xIntf *intf, const Dot1xCak *cak);

/*
 * Detach a CAK from associated objects.
 */
static void
detach_cak(Dot1xCak *cak);

/*
 * Detach a PACP from its CAK and try reattaching to the distributed
 * group CAK if such exists and the PACP was not already attached to
 * it.
 */
static void
detach_pacp(Dot1xPacp *pacp);

/*
 * Test CKN derivation using test vectors. Return 1 if successful.
 */
static int
test_ckn_derivation(void);

/*
 * Test CAK derivation using test vectors. Return 1 if successful.
 */
static int
test_cak_derivation(void);

/*
 * Add session id and MAC address information to a CMAC for a CKN
 * generated from a MSK. The `arg' parameter points to a PACP
 * instance.
 */
static void
context_ckn(void *arg, Dot1xCryptoCmac *cmac);

/*
 * Add MAC address information to a CMAC for a CAK generated from a
 * MSK. The `arg' parameter points to a PACP instance.
 */
static void
context_cak(void *arg, Dot1xCryptoCmac *cmac);

/*
 * Public functions.
 */

int
dot1x_step_logon(Dot1xIntf *intf)
{
  /* create/destroy MKA instances for CAKs */
  dot1x_caks_foreach(&intf->caks, step_cak);

  /* control PACP */
  dot1x_foreach_port(intf, step_authenticate);

  /* control CP */
  dot1x_foreach_port(intf, step_connect);

  if (intf->logon_changed)
    {
      intf->logon_changed = 0;
      return 1;
    }

  return 0;
}

void
dot1x_tick_logon(Dot1xIntf *intf)
{
  dot1x_caks_foreach(&intf->caks, tick_cak);
}

void
dot1x_process_eap_results(
  Dot1xPacp *pacp, const MacsecSessionId *session_id, const MacsecMsk *msk,
  const MacsecAuthData *auth_data)
{
  Dot1xIntf *intf = pacp->side->intf;
  KeyDerivationArg kda;
  Dot1xCak *cak;
  Dot1xCkn ckn;
  Dot1xCakType type;
  Dot1xNid *nid;
  MacsecKey key;
  MacsecKey msk0;
  Dot1xFormatBuffer fb;

  /* create a new CAK only if there is a valid MSK and session id */
  if (session_id->len <= 0 || msk->len <= 0)
    return;

  /* detach from possible existing CAK */
  /* if it is not a distributed group CAK, make it expire soon */
  if (pacp->cak)
    {
      if (pacp->cak->type != DOT1X_CAK_DISTRIBUTED)
        pacp->cak->life_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
      dot1x_unlink_pacp_cak(pacp);
    }

  if (!intf->ctrl.long_eap_caks)
    msk0.len = 16;
  else
    msk0.len = 32;
  memcpy(msk0.buf, msk->buf, msk0.len);

  /* derive CKN and CAK */
  kda.intf = intf;
  kda.partner = &pacp->partner;
  kda.session_id = session_id;
  ckn.len = msk0.len;
  dot1x_derive_key(
    intf, ckn.buf, ckn.len, &msk0, "IEEE8021 EAP CKN",
    context_ckn, &kda);
  key.len = msk0.len;
  dot1x_derive_key(
    intf, key.buf, key.len, &msk0, "IEEE8021 EAP CAK",
    context_cak, &kda);

  /* clear MSK in stack */
  memset(&msk0, 0, sizeof msk0);

  type = DOT1X_CAK_EAP;
  nid = &pacp->network->nid;

  /* remove disabled CAKs replaced by the new one */
  dot1x_remove_disabled_caks(intf, type, nid);

  /* create a new CAK with the NID EAP was using */
  if (!(cak = dot1x_insert_cak(intf, &ckn, &key, type, nid)))
    return;
  dot1x_adcpy(&cak->auth_data, auth_data);

  /* mark supplicant or authenticator role (for key server elections) */
  if (pacp->side == &intf->supp)
    cak->supplicant = 1;
  else
    cak->authenticator = 1;

  /* equip authenticator's CAK with KMD, to be conveyed to supplicant by MKA */
  if (cak->authenticator)
    dot1x_kmdcpy(&cak->kmd, &pacp->network->ctrl.kmd);

  cak->partner = pacp->partner;
  cak->has_partner = 1;

  /* if PACP is authenticator with reauth enabled set finite key life */
  /* `finite' meaning reauth period plus ten seconds */
  if (pacp->side == &intf->auth && intf->ctrl.reauth_enabled)
    cak->life_ticks = (intf->ctrl.reauth_period + 10) * DOT1X_TICKS_PER_SECOND;
  else
    cak->life_ticks = 0;

  /* double-link PACP and CAK */
  dot1x_link_pacp_cak(pacp, cak);

  dot1x_debug_cak(cak, "derived from %s", dot1x_format_pacp(&fb, pacp));

  if (intf->cak_cache_callback)
    intf->cak_cache_callback(DOT1X_CAK_CACHE_INSERT, cak);
}

void
dot1x_process_eap_failure(Dot1xPacp *pacp)
{
  Dot1xPort *port = pacp->port;
  Dot1xCak *badcak;

  if (pacp->cak)
    badcak = pacp->cak;
  else
  if (port->distributed_cak &&
      pacp->network == port->distributed_cak->network)
    badcak = port->distributed_cak;
  else
    return;

  if (!badcak->eapfail)
    {
      dot1x_debug_cak(badcak, "marked bad because of EAP failure");
      badcak->eapfail = 1;
      badcak->life_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
    }
}

void
dot1x_link_cak_network(Dot1xCak *cak, Dot1xNetwork *netw)
{
  Dot1xFormatBuffer fb;

  cak->network = netw;
  macsec_list_append(&netw->caks, &cak->network_link);
  dot1x_debug_cak(cak, "associated with %s", dot1x_format_network(&fb, netw));
}

void
dot1x_unlink_cak_network(Dot1xCak *cak)
{
  macsec_list_remove(&cak->network_link);
  cak->network = NULL;
}

void
dot1x_remove_disabled_caks(
  Dot1xIntf *intf, Dot1xCakType type, const Dot1xNid *nid)
{
  Dot1xCak *cak, *next;

  next = dot1x_caks_next(&intf->caks, NULL);
  while ((cak = next))
    {
      next = dot1x_caks_next(&intf->caks, cak);

      if (cak->enable || cak->type != type || dot1x_nidcmp(&cak->nid, nid))
        continue;

      dot1x_remove_cak(cak);
    }
}

void
dot1x_refresh_caks(Dot1xIntf *intf, const Dot1xNid *nid, const Dot1xKmd *kmd)
{
  Dot1xCak *cak;

  if (kmd->len <= 0)
    return;

  cak = NULL;
  while ((cak = dot1x_caks_next(&intf->caks, cak)))
    {
      if (cak->preshared)
        continue;

      if (cak->enable)
        continue;

      if (dot1x_kmdcmp(&cak->kmd, kmd))
        continue;

      if (dot1x_nidcmp(&cak->nid, nid))
        continue;

      cak->active_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
      intf->changed = 1;
    }
}

void
dot1x_refresh_cak(Dot1xCak *cak)
{
  if (cak->enable)
    return;

  cak->active_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
}

void
dot1x_inherit_cak(Dot1xCak *dst, const Dot1xCak *src)
{
  dst->port = src->port;
  dot1x_reref_port(src->port);
  dst->supplicant = src->supplicant;
  dst->authenticator = src->authenticator;
  dot1x_link_cak_network(dst, src->network);
  dot1x_adcpy(&dst->auth_data, &src->auth_data);
  dot1x_kmdcpy(&dst->kmd, &src->kmd);
}

Dot1xCak *
dot1x_insert_cak(
  Dot1xIntf *intf, const Dot1xCkn *ckn, const MacsecKey *key,
  Dot1xCakType type, const Dot1xNid *nid)
{
  Dot1xCak *cak;
  Dot1xFormatBuffer fb;

  if (dot1x_caks_lookup(&intf->caks, ckn))
    {
      dot1x_error(intf, "CAK with matching CKN already exists");
      return NULL;
    }

  if (!(cak = dot1x_caks_insert(&intf->caks, ckn)))
    {
      dot1x_error(intf, "out of CAK entries");
      return NULL;
    }
  cak->intf = intf;
  macsec_list_init(&cak->pacps);
  dot1x_keycpy(&cak->key, key);
  cak->type = type;
  if (type == DOT1X_CAK_PAIRWISE || type == DOT1X_CAK_EAP)
    cak->pairwise = 1;
  if (type == DOT1X_CAK_GROUP || type == DOT1X_CAK_PAIRWISE)
    cak->preshared = 1;
  dot1x_nidcpy(&cak->nid, nid);
  cak->enable = 1;

  /* mark pre-shared CAKs */
  if (type == DOT1X_CAK_GROUP || type == DOT1X_CAK_PAIRWISE)
    cak->preshared = 1;
  /* mark pairwise CAKs */
  if (type == DOT1X_CAK_PAIRWISE || type == DOT1X_CAK_EAP)
    cak->pairwise = 1;

  dot1x_debug_cak(
    cak, "created with CKN %s", dot1x_format_ckn(&fb, &cak->ckn));
  dot1x_debug_cak(cak, "length is %u bits", cak->key.len << 3);
  if (cak->nid.len > 0)
    dot1x_debug_cak(cak, "has NID %s", dot1x_format_nid(&fb, &cak->nid));

  return cak;
}

void
dot1x_remove_cak(Dot1xCak *cak)
{
  Dot1xIntf *intf = cak->intf;

  assert(cak->intf != NULL);
  dot1x_debug_cak(cak, "being removed");

  if (intf->cak_cache_callback)
      intf->cak_cache_callback(DOT1X_CAK_CACHE_DELETE, cak);

  detach_cak(cak);
  if (dot1x_caks_remove(&intf->caks, &cak->ckn) != cak)
      assert(0);

  memset(cak, 0, sizeof *cak);
}

int
dot1x_test_logon(void)
{
  int ok = 1;

  ok &= test_ckn_derivation();
  ok &= test_cak_derivation();
  return ok;
}

/*
 * Static functions.
 */

static void
tick_cak(Dot1xCak *cak)
{
  if (cak->life_ticks > 1 && --cak->life_ticks == 1)
    dot1x_debug_cak(cak, "expired");

  if (cak->active_ticks > 1 && --cak->active_ticks == 1)
    dot1x_debug_cak(cak, "is inactive");

  if (cak->held_while_ticks)
    cak->held_while_ticks--;

  if (cak->replace_ticks)
    cak->replace_ticks--;

  if (cak->activated_callback_ticks > 1)
    cak->activated_callback_ticks--;
}

static void
step_cak(Dot1xCak *cak)
{
  Dot1xFormatBuffer fb;
  Dot1xIntf *intf = cak->intf;
  Dot1xNetwork *netw;
  unsigned int enable, participate;

  /* destroy non-preshared CAK if the associated EAP side has been
     disabled */
  if ((cak->supplicant && !intf->ctrl.supplicant_enable) ||
      (cak->authenticator && !intf->ctrl.authenticator_enable))
    {
      dot1x_remove_cak(cak);
      intf->logon_changed = 1;
      return;
    }

  /* determine desired CAK enable state */
  enable =
    /* logged on */
    intf->ctrl.logon &&
    /* uncontrolled port enabled */
    intf->port_enabled &&
    /* CAK has not become inactive */
    cak->active_ticks != 1 &&
    /* CAK life is not expired */
    cak->life_ticks != 1;

  if (enable && !cak->enable)
    {
      cak->enable = 1;
      dot1x_debug_cak(cak, "enabled");
    }
  else if (!enable && cak->enable)
    {
      dot1x_debug_cak(cak, "being disabled");
      detach_cak(cak);
      cak->enable = 0;
    }

  /* destroy non-preshared CAK if disabled and cannot be cached */
  if (!cak->preshared && !cak->enable &&
      (cak->life_ticks == 1 ||
       !intf->ctrl.cak_caching_enable ||
       cak->kmd.len <= 0 ||
       (!cak->pairwise && !intf->ctrl.group_ca_enable) ||
       find_better_cak(intf, cak)))
    {
      dot1x_remove_cak(cak);
      intf->logon_changed = 1;
      return;
    }

  /* limit life of non-preshared CAKs without EAP failures but without
     associated PACPs to reAuthPeriod */
  if (!cak->preshared && !cak->eapfail)
    {
      if (!cak->life_ticks && macsec_list_empty(&cak->pacps))
        cak->life_ticks = intf->ctrl.reauth_period * DOT1X_TICKS_PER_SECOND;
      else if (cak->life_ticks && !macsec_list_empty(&cak->pacps))
        cak->life_ticks = 0;
    }

  /* determine desired MKA participation state */
  participate =
    /* MKA enabled */
    intf->mka_enabled &&
    /* CAK enabled */
    cak->enable &&
    /* CAK is not being held down after failure */
    !cak->held_while_ticks &&
    /* if virtual ports are enabled CAK must have peer address */
    (!intf->ctrl.virtual_ports_enable || cak->has_partner) &&
    /* virtual port enable control does not require destroying port */
    (!cak->port ||
     (!intf->ctrl.virtual_ports_enable && cak->port == &intf->rport) ||
     (intf->ctrl.virtual_ports_enable && cak->port != &intf->rport));

  /* get network if needed */
  if (!cak->network && participate)
    {
      if (cak->nid.len <= 0)
        netw = &intf->dnetwork;
      else
        netw = dot1x_networks_lookup(&intf->networks, &cak->nid);
      if (netw)
        dot1x_link_cak_network(cak, netw);
      else
        {
          dot1x_debug_cak(cak, "without network");
          /* retry network inquiry after a while */
          cak->held_while_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
        }
    }

  /* do not participate if selected network conflicts with CAK NID */
  if (participate &&
      intf->snetwork && cak->network &&
      cak->network != intf->snetwork)
    participate = 0;

  /* get port if needed */
  if (!cak->port && participate)
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

  /* create MKA instance if needed */
  if (cak->network && cak->port && !cak->mka && participate)
    {
      dot1x_debug_logon_mka(cak->port, "createMKA()");

      if (!(cak->mka = dot1x_create_mka(cak)))
        {
          /* retry MKA creation after a while */
          cak->held_while_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
          dot1x_debug_cak(cak, "being held down for a while");
        }
      intf->logon_changed = 1;
    }

  /* destroy MKA instance if needed */
  if (cak->port && cak->mka &&
      (!cak->network ||
       !participate ||
       cak->mka->duplicate ||
       cak->mka->mn >= (uint32_t)-1 ||
       cak->mka->kn >= (uint32_t)-1))
    {
      /* after address/MI collision hold down the CAK for a while */
      if (cak->mka && cak->mka->duplicate)
        cak->held_while_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;

      dot1x_debug_logon_mka(cak->port, "MKA.delete()");
      if (cak->replaced_mka)
        {
          dot1x_destroy_mka(cak->replaced_mka);
          cak->replaced_mka = NULL;
          cak->replace_ticks = 0;
        }
      dot1x_destroy_mka(cak->mka);
      cak->mka = NULL;
      intf->logon_changed = 1;
    }

  /* start another MKA instance for replacing the current one if needed */
  if (cak->mka && cak->mka->replace && !cak->replaced_mka)
    {
      cak->replaced_mka = cak->mka;
      cak->replace_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
      if (!(cak->mka = dot1x_create_mka(cak)))
        {
          /* retry MKA creation after a while */
          cak->held_while_ticks = DOT1X_MKA_LIFE_TIME * DOT1X_TICKS_PER_SECOND;
          dot1x_debug_cak(cak, "being held down for a while");
        }
      intf->logon_changed = 1;
    }

  /* destroy replaced MKA instance */
  if (cak->replaced_mka && !cak->replace_ticks)
    {
      dot1x_destroy_mka(cak->replaced_mka);
      cak->replaced_mka = NULL;
    }

  /* release port if needed */
  if (cak->port && !participate)
    {
      dot1x_unref_port(cak->port);
      if (cak->port->authorization_data == &cak->auth_data)
        cak->port->authorization_data = NULL;
      cak->port = NULL;
      intf->logon_changed = 1;
    }

  /* report CAK activated (after a timeout) if callback configured 
   * and received/sent distributed CAK/SAK */
  if (cak->port &&
      cak->mka &&
      cak->preshared &&
      cak->activated_callback &&
      cak->sent_or_received_cak_or_sak &&
      !cak->reported_activated)
    {
      if (cak->activated_callback_ticks == 0)
        cak->activated_callback_ticks = DOT1X_MKA_LIFE_TIME * 
          DOT1X_TICKS_PER_SECOND;

      if (cak->activated_callback_ticks == 1)
        {
          dot1x_debug_cak(cak, "CAK activity: CKN %s -> ACTIVATED", 
            dot1x_format_ckn(&fb, &cak->ckn));
          cak->reported_activated = 1;
          cak->activated_callback(cak);
        }
    }
}

static void
step_authenticate(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;
  Dot1xNetwork *netw = dot1x_choose_network(port);
  int supp_authenticate, auth_authenticate;

  if (intf->ctrl.logon &&
      (netw->ctrl.use_eap == DOT1X_USE_EAP_IMMEDIATE ||
       (!intf->ctrl.virtual_ports_enable &&
        netw->ctrl.use_eap == DOT1X_USE_EAP_MKA_FAIL &&
        (port->mka_failed || !intf->mka_enabled))))
    {
      supp_authenticate = intf->ctrl.supplicant_enable;
      auth_authenticate = intf->ctrl.authenticator_enable;
    }
  else
    {
      supp_authenticate = 0;
      auth_authenticate = 0;
    }

  if (port->supp_authenticate != supp_authenticate)
    {
      port->supp_authenticate = supp_authenticate;
      dot1x_debug_logon_pacp_port(
        port, "supp.authenticate = %s", dot1x_bool(port->supp_authenticate));
      intf->logon_changed = 1;
    }

  if (port->auth_authenticate != auth_authenticate)
    {
      port->auth_authenticate = auth_authenticate;
      dot1x_debug_logon_pacp_port(
        port, "auth.authenticate = %s", dot1x_bool(port->auth_authenticate));
      intf->logon_changed = 1;
    }
}

static void
step_connect(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;
  Dot1xNetwork *mkanetw, *eapnetw, *netw, *cnetwork;
  Dot1xFormatBuffer fb;
  int connect;

  /* clear saved EAP failures on port down or logon false */
  if (intf->supp.failed_save && (!intf->port_enabled || !intf->ctrl.logon))
    intf->supp.failed_save = 0;
  if (intf->auth.failed_save && (!intf->port_enabled || !intf->ctrl.logon))
    intf->auth.failed_save = 0;

  /* save EAP failures from signals that are going to be reset after use */
  if (intf->supp.failed)
    intf->supp.failed_save = 1;
  if (intf->auth.failed)
    intf->auth.failed_save = 1;

  if (port->principal)
    mkanetw = port->principal->cak->network;
  else
    mkanetw = NULL;

  if (intf->supp.last_authenticated_pacp)
    eapnetw = intf->supp.last_authenticated_pacp->network;
  else if (intf->auth.last_authenticated_pacp)
    eapnetw = intf->auth.last_authenticated_pacp->network;
  else
    eapnetw = NULL;

  netw = dot1x_choose_network(port);

  if (!intf->port_enabled)
    {
      connect = DOT1X_CONNECT_PENDING;
      cnetwork = NULL;
    }
  else
  if (intf->ctrl.logon &&
      mkanetw &&
      port->mka_secured)
    {
      connect = DOT1X_CONNECT_SECURE;
      cnetwork = mkanetw;
    }
  else
  if (intf->ctrl.logon &&
      mkanetw &&
      mkanetw->ctrl.unsecured_allowed == DOT1X_UNSECURED_ALLOWED_MKA_SERVER &&
      port->mka_authenticated)
    {
      connect = DOT1X_CONNECT_AUTHENTICATED;
      cnetwork = mkanetw;
    }
  else
  if (intf->ctrl.logon &&
      eapnetw &&
      !intf->ctrl.virtual_ports_enable &&

      ((eapnetw->ctrl.unsecured_allowed == DOT1X_UNSECURED_ALLOWED_IMMEDIATE &&
        (intf->supp.authenticated || intf->auth.authenticated)) ||

       (eapnetw->ctrl.unsecured_allowed == DOT1X_UNSECURED_ALLOWED_MKA_FAIL &&
        (intf->supp.authenticated || intf->auth.authenticated) &&
        (port->mka_failed || !intf->mka_enabled))))
    {
      connect = DOT1X_CONNECT_AUTHENTICATED;
      cnetwork = eapnetw;
    }
  else
  if (!intf->ctrl.virtual_ports_enable &&

      (netw->ctrl.unauth_allowed == DOT1X_UNAUTH_ALLOWED_IMMEDIATE ||

       (netw->ctrl.unauth_allowed == DOT1X_UNAUTH_ALLOWED_AUTH_FAIL &&
        intf->ctrl.logon &&
        (intf->supp.failed_save || !intf->supp.enabled) &&
        (intf->auth.failed_save || !intf->auth.enabled) &&
        (port->mka_failed || !intf->mka_enabled ||
         !dot1x_mkas_next(&intf->mkas, NULL)))))
    {
      connect = DOT1X_CONNECT_UNAUTHENTICATED;
      cnetwork = netw;
    }
  else
    {
      connect = DOT1X_CONNECT_PENDING;
      cnetwork = NULL;
    }

  if (connect != port->connect)
    {
      port->connect = connect;
      dot1x_debug_logon_cp(port, "connect = %s", dot1x_connect(port->connect));
      intf->logon_changed = 1;
    }

  if (cnetwork != port->cnetwork)
    {
      if (port->cnetwork)
        dot1x_debug_network_port(
          port, "disconnected from %s",
          dot1x_format_network(&fb, port->cnetwork));
      if (cnetwork)
        dot1x_debug_network_port(
          port, "connected to %s", dot1x_format_network(&fb, cnetwork));
      port->cnetwork = cnetwork;
      /* XXX connectedNID should be per-port */
      if (port->cnetwork)
        dot1x_nidcpy(&intf->stat.connected_nid, &port->cnetwork->nid);
      else
        intf->stat.connected_nid.len = 0;
    }

  /* consume the failure signals from the PACP machines */
  if (intf->supp.failed)
    {
      intf->supp.failed = 0;
      dot1x_debug_logon_pacp(
        intf, "supp.failed = %s", dot1x_bool(intf->supp.failed));
      intf->logon_changed = 1;
    }
  if (intf->auth.failed)
    {
      intf->auth.failed = 0;
      dot1x_debug_logon_pacp(
        intf, "auth.failed = %s", dot1x_bool(intf->auth.failed));
      intf->logon_changed = 1;
    }
}

static Dot1xCak *
find_better_cak(Dot1xIntf *intf, const Dot1xCak *cak)
{
  Dot1xCak *other;

  other = NULL;
  while ((other = dot1x_caks_next(&intf->caks, other)))
    {
      if (other == cak)
        continue;

      if (!other->enable)
        continue;

      if (!cak->pairwise && other->pairwise)
        continue;

      if (!dot1x_nidcmp(&other->nid, &cak->nid))
        return other;
    }

  return NULL;
}

static void
detach_cak(Dot1xCak *cak)
{
  /* stop being the distributed CAK */
  if (cak->port && cak->port->distributed_cak == cak)
    cak->port->distributed_cak = NULL;

  /* destroy MKA actor(s) */
  if (cak->port && cak->mka)
    {
      dot1x_debug_logon_mka(cak->port, "MKA.delete()");
      if (cak->replaced_mka)
        {
          dot1x_destroy_mka(cak->replaced_mka);
          cak->replaced_mka = NULL;
        }
      dot1x_destroy_mka(cak->mka);
      cak->mka = NULL;
    }

  /* unlink network */
  if (cak->network)
    dot1x_unlink_cak_network(cak);

  /* release port */
  if (cak->port)
    {
      dot1x_unref_port(cak->port);
      if (cak->port->authorization_data == &cak->auth_data)
        cak->port->authorization_data = NULL;
      cak->port = NULL;
    }

  /* clear partner information */
  memset(&cak->partner, 0, sizeof cak->partner);
  cak->has_partner = 0;

  /* detach possible attached PACPs */
  macsec_list_foreach(&cak->pacps, offsetof(Dot1xPacp, cak_link), detach_pacp);
}

static void
detach_pacp(Dot1xPacp *pacp)
{
  Dot1xPort *port = pacp->port;
  Dot1xCak *cak = pacp->cak;

  if (cak)
    dot1x_unlink_pacp_cak(pacp);

  /* try re-attaching to a distributed group CAK if possible */
  if (port->distributed_cak &&
      cak != port->distributed_cak &&
      pacp->network == port->distributed_cak->network)
    dot1x_link_pacp_cak(pacp, port->distributed_cak);
}

static int
test_ckn_derivation(void)
{
  const unsigned char test_key[] = {
    0xe6, 0x8a, 0x1a, 0xb9, 0x03, 0x13, 0x02, 0x4f,
    0xda, 0x7a, 0x04, 0xa0, 0x3f, 0xea, 0x01, 0x0f};
  const unsigned char test_label[] = {
    0x49, 0x45, 0x45, 0x45, 0x38, 0x30, 0x32, 0x31,
    0x20, 0x45, 0x41, 0x50, 0x20, 0x43, 0x4b, 0x4e};
  const unsigned char test_session_id[] = {
    0x0d,
    0xd0, 0x75, 0x69, 0x3f, 0x54, 0xb2, 0xb2, 0xeb,
    0x01, 0xda, 0x61, 0xf0, 0xaf, 0x5d, 0x42, 0x9b,
    0x65, 0xb1, 0xeb, 0xca, 0xf5, 0x36, 0xfb, 0xa3,
    0x50, 0x77, 0x75, 0x98, 0x57, 0x17, 0x28, 0xf6,
    0x30, 0xc5, 0xc8, 0xda, 0x54, 0x75, 0x48, 0x9a,
    0xd4, 0x7b, 0x6e, 0x34, 0x89, 0xa9, 0x73, 0x72,
    0xe5, 0xa8, 0xfd, 0x55, 0x06, 0x17, 0x97, 0x2c,
    0x02, 0x0d, 0x42, 0xa3, 0xb1, 0x3a, 0x4e, 0xae};
  const unsigned char test_mac1[] = {
    0x00, 0x1b, 0x63, 0x93, 0xfc, 0xbc};
  const unsigned char test_mac2[] = {
    0x00, 0xd0, 0xb7, 0x1a, 0x77, 0x17};
  const unsigned char test_length[] = {
    0x00, 0x80};
  const unsigned char test_output[] = {
    0x96, 0x43, 0x7a, 0x93, 0xcc, 0xf1, 0x0d, 0x9d,
    0xfe, 0x34, 0x78, 0x46, 0xcc, 0xe5, 0x2c, 0x7d};

  MacsecSessionId session_id;
  KeyDerivationArg kda;
  Dot1xIntf intf;
  MacsecKey key;
  Dot1xCkn ckn;

  memcpy(intf.local_address.buf, test_mac1, 6);
  assert(sizeof test_session_id == sizeof session_id.buf);
  session_id.len = sizeof test_session_id;
  memcpy(session_id.buf, test_session_id, session_id.len);

  kda.intf = &intf;
  kda.partner = (void *)&test_mac2;
  kda.session_id = &session_id;

  memcpy(key.buf, test_key, sizeof test_key);
  key.len = sizeof test_key;

  assert(((test_length[0] << 8) | test_length[1]) == (16 << 3));
  ckn.len = 16;

  assert(sizeof test_label == 16);
  assert(!memcmp(test_label, "IEEE8021 EAP CKN", 16));

  dot1x_derive_key(
    &intf, ckn.buf, ckn.len, &key, "IEEE8021 EAP CKN",
    context_ckn, &kda);

  assert(sizeof test_output == ckn.len);
  if (memcmp(ckn.buf, test_output, ckn.len))
    return 0;

  return 1;
}

static int
test_cak_derivation(void)
{
  const unsigned char test_key[] = {
    0xe6, 0x8a, 0x1a, 0xb9, 0x03, 0x13, 0x02, 0x4f,
    0xda, 0x7a, 0x04, 0xa0, 0x3f, 0xea, 0x01, 0x0f};
  const unsigned char test_label[] = {
    0x49, 0x45, 0x45, 0x45, 0x38, 0x30, 0x32, 0x31,
    0x20, 0x45, 0x41, 0x50, 0x20, 0x43, 0x41, 0x4b};
  const unsigned char test_context[] = {
    0x00, 0x1b, 0x63, 0x93, 0xfc, 0xbc, 0x00, 0xd0,
    0xb7, 0x1a, 0x77, 0x17};
  const unsigned char test_length[] = {
    0x00, 0x80};
  const unsigned char test_output[] = {
    0x13, 0x5b, 0xd7, 0x58, 0xb0, 0xee, 0x5c, 0x11,
    0xc5, 0x5f, 0xf6, 0xab, 0x19, 0xfd, 0xb1, 0x99};

  KeyDerivationArg kda;
  Dot1xIntf intf;
  MacsecKey key;
  MacsecKey cak;

  assert(sizeof test_context == 12);
  memcpy(intf.local_address.buf, test_context, 6);

  kda.intf = &intf;
  kda.partner = (void *)(test_context + 6);

  memcpy(key.buf, test_key, sizeof test_key);
  key.len = sizeof test_key;

  assert(((test_length[0] << 8) | test_length[1]) == (16 << 3));
  cak.len = 16;

  assert(sizeof test_label == 16);
  assert(!memcmp(test_label, "IEEE8021 EAP CAK", 16));

  dot1x_derive_key(
    &intf, cak.buf, cak.len, &key, "IEEE8021 EAP CAK",
    context_cak, &kda);

  assert(sizeof test_output == cak.len);
  if (memcmp(cak.buf, test_output, cak.len))
    return 0;

  return 1;
}

static void
context_ckn(void *arg, Dot1xCryptoCmac *cmac)
{
  const KeyDerivationArg *kda = arg;
  const Dot1xIntf *intf = kda->intf;
  const MacsecAddress *mac1, *mac2;

  dot1x_crypto_cmac_update(
    cmac, kda->session_id->buf, sizeof kda->session_id->buf);

  if (memcmp(kda->partner, &intf->local_address, sizeof *kda->partner) < 0)
    {
      mac1 = kda->partner;
      mac2 = &intf->local_address;
    }
  else
    {
      mac1 = &intf->local_address;
      mac2 = kda->partner;
    }
  dot1x_crypto_cmac_update(cmac, mac1->buf, sizeof mac1->buf);
  dot1x_crypto_cmac_update(cmac, mac2->buf, sizeof mac2->buf);
}

static void
context_cak(void *arg, Dot1xCryptoCmac *cmac)
{
  const KeyDerivationArg *kda = arg;
  const Dot1xIntf *intf = kda->intf;
  const MacsecAddress *mac1, *mac2;

  if (memcmp(kda->partner, &intf->local_address, sizeof *kda->partner) < 0)
    {
      mac1 = kda->partner;
      mac2 = &intf->local_address;
    }
  else
    {
      mac1 = &intf->local_address;
      mac2 = kda->partner;
    }
  dot1x_crypto_cmac_update(cmac, mac1->buf, sizeof mac1->buf);
  dot1x_crypto_cmac_update(cmac, mac2->buf, sizeof mac2->buf);
}
