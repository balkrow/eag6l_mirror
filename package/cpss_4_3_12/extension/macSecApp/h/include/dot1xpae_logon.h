/**

   IEEE 802.1X-2010 Logon process.

   File: dot1xpae_logon.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_LOGON_H
#define _DOT1XPAE_LOGON_H

#include "dot1xpae_types.h"

/*
 * Update the state of the Logon process. Return nonzero if it changed
 * any conditions that might affect other state machines.
 */
int
dot1x_step_logon(Dot1xIntf *intf);

/*
 * Update timers of the Logon process.
 */
void
dot1x_tick_logon(Dot1xIntf *intf);

/*
 * Create a CAK from EAP results and link it with the given PACP
 * object, provided both session identifier and MSK are valid.
 */
void
dot1x_process_eap_results(
  Dot1xPacp *pacp, const MacsecSessionId *session_id, const MacsecMsk *msk,
  const MacsecAuthData *auth_data);

/*
 * Handle EAP authentication failure. In case of reauthentication
 * failure with a PACP currently associated with a CAK, make the CAK
 * expire soon, even if it is a group CAK associated with multiple
 * PACPs. */
void
dot1x_process_eap_failure(Dot1xPacp *pacp);

/*
 * Link CAK with a network and vice versa.
 */
void
dot1x_link_cak_network(Dot1xCak *cak, Dot1xNetwork *netw);

/*
 * Break link between a CAK and the network it was linked with.
 */
void
dot1x_unlink_cak_network(Dot1xCak *cak);

/*
 * Remove disabled CAKs with matching type and NID.
 */
void
dot1x_remove_disabled_caks(
  Dot1xIntf *intf, Dot1xCakType type, const Dot1xNid *nid);

/*
 * Lookup disabled CAKs by NID and KMD and enable them.
 */
void
dot1x_refresh_caks(Dot1xIntf *intf, const Dot1xNid *nid, const Dot1xKmd *kmd);

/*
 * Enable a specific disabled CAK.
 */
void
dot1x_refresh_cak(Dot1xCak *cak);

/*
 * Make port, network, auth_data and KMD of a CAK equivalent to those
 * of another CAK.
 */
void
dot1x_inherit_cak(Dot1xCak *dst, const Dot1xCak *src);

/*
 * Try creating a CAK.
 */
Dot1xCak *
dot1x_insert_cak(
  Dot1xIntf *intf, const Dot1xCkn *ckn,
  const MacsecKey *key, Dot1xCakType type, const Dot1xNid *nid);

/*
 * Clean up and remove a CAK.
 */
void
dot1x_remove_cak(Dot1xCak *cak);

/*
 * Run test vectors through CKN and CAK derivation functions. Return
 * 1 if successful, 0 otherwise.
 */
int
dot1x_test_logon(void);

#endif /* _DOT1XPAE_LOGON_H */
