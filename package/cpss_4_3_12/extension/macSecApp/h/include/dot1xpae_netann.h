/**

   IEEE 802.1X-2010 network announcement support.

   File: dot1xpae_netann.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_NETANN_H
#define _DOT1XPAE_NETANN_H

#include "dot1xpae_types.h"

/*
 * Create a network object and return pointer to it, NULL otherwise.
 */
Dot1xNetwork *
dot1x_insert_network(Dot1xIntf *intf, const Dot1xNid *nid);

/*
 * Destroy a network object.
 */
void
dot1x_remove_network(Dot1xNetwork *n);

/*
 * Update the state of the announcer. Return nonzero if it changed any
 * conditions that might affect other state machines.
 */
int
dot1x_step_netann(Dot1xIntf *intf);

/*
 * Update timers of the announcer.
 */
void
dot1x_tick_netann(Dot1xIntf *intf);

/*
 * Decode and process an EAPOL-Announcement packet.
 */
void
dot1x_receive_announcement(
  Dot1xIntf *intf, const MacsecAddress *dst, unsigned type,
  const unsigned char *buf, unsigned len);

/*
 * Decode and process and EAPOL-Announcement-Req packet.
 */
void
dot1x_receive_announcement_req(
  Dot1xIntf *intf, const MacsecAddress *src, const unsigned char *buf,
  unsigned len);

/*
 * Generate an announcement to be sent in a generic or specific
 * EAPOL-Announcement or EAPOL-MKA packet.
 */
void
dot1x_generate_announcement(Dot1xPort *port, Dot1xAnpdu *ann);

/*
 * Process an announcement received in a generic or specific
 * EAPOL-Announcement or EAPOL-MKA packet.
 */
void
dot1x_process_announcement(
  Dot1xIntf *intf, unsigned eapol_type, const Dot1xAnpdu *ann);

/*
 * Generate an announcement request to be sent in an
 * EAPOL-Announcement-Req or EAPOL-Start packet.
 */
void
dot1x_generate_announcement_req(Dot1xIntf *intf, Dot1xAnpduReq *req);

/*
 * Process an announcement request received in an
 * EAPOL-Announcement-Req or EAPOL-Start packet.
 */
void
dot1x_process_announcement_req(Dot1xPort *port, const Dot1xAnpduReq *req);

/*
 * Default network control values.
 */
extern const Dot1xNetworkCtrl dot1x_default_network_ctrl;

#endif /* _DOT1XPAE_NETANN_H */
