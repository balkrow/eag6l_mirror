/**

   IEEE 802.1X-2010 PACP state machines.

   File: dot1xpae_pacp.h

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_PACP_H
#define _DOT1XPAE_PACP_H

#include "dot1xpae_types.h"

/*
 * Link PACP instance with a network and vice versa.
 */
void
dot1x_link_pacp_network(Dot1xPacp *pacp, Dot1xNetwork *netw);

/*
 * Break link between a PACP instance and the network it was linked
 * with.
 */
void
dot1x_unlink_pacp_network(Dot1xPacp *pacp);

/*
 * Link PACP instance with a CAK and vice versa.
 */
void
dot1x_link_pacp_cak(Dot1xPacp *pacp, Dot1xCak *cak);

/*
 * Break link between a PACP instance and the CAK it was linked with.
 */
void
dot1x_unlink_pacp_cak(Dot1xPacp *pacp);

/*
 * Create a PACP instance in an EAP side (authenticator or supplicant)
 * of an interface and associated with the specified partner MAC
 * address. The address must be valid; to indicate no known partner
 * use an all-zero address.
 */
Dot1xPacp *
dot1x_create_pacp(Dot1xPacpSide *side, const MacsecAddress *partner);

/*
 * Destroy a PACP instance.
 */
void
dot1x_destroy_pacp(Dot1xPacp *pacp);

/*
 * Run all PACP state machines of the interface. Return 1 if it
 * changed any conditions that might affect other state machines.
 */
int
dot1x_step_pacp(Dot1xIntf *intf);

/*
 * Update PACP timers.
 */
void
dot1x_tick_pacp(Dot1xIntf *intf);

/*
 * Process an EAP message.
 */
void
dot1x_receive_eap_msg(
  Dot1xIntf *intf, const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *buf, unsigned len);

/*
 * Process an EAPOL-Start packet.
 */
void
dot1x_receive_start(
  Dot1xIntf *intf, unsigned version, const MacsecAddress *src,
  const unsigned char *buf, unsigned len);

/*
 * Process an EAPOL-Logoff packet.
 */
void
dot1x_receive_logoff(Dot1xIntf *intf, const MacsecAddress *src);

void
dot1x_pacp_add_from_cache(Dot1xIntf *intf, Dot1xCak *cak);

#endif /* _DOT1XPAE_PACP_H */
