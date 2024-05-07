/**

   IEEE 802.1X-2010 MKA protocol.

   File: dot1xpae_mka.h

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_MKA_H
#define _DOT1XPAE_MKA_H

#include "dot1xpae_types.h"

/*
 * MKA time values in seconds except bounded hello in milliseconds.
 */
#define DOT1X_MKA_LIFE_TIME 6
#define DOT1X_MKA_HELLO_TIME 2
#define DOT1X_MKA_SUSPENSION_LIMIT 120
#define DOT1X_MKA_BOUNDED_HELLO_MILLISECONDS 500

/*
 * Create an MKA instance with the given CAK. Return pointer to the
 * created instance if successful, NULL otherwise. The instance
 * participates in the MKA protocol immediately after creation.
 */
Dot1xMka *
dot1x_create_mka(Dot1xCak *cak);

/*
 * Destroy an MKA instance.
 */
void
dot1x_destroy_mka(Dot1xMka *mka);

/*
 * Advance the MKA timers of the interface.
 */
void
dot1x_tick_mka(Dot1xIntf *intf);

/*
 * Run the MKA entity of the interface. Return 1 if it changed any
 * conditions that might affect other state machines, 0 otherwise
 */
int
dot1x_step_mka(Dot1xIntf *intf);

/*
 * Send MKPDUs for MKA instance marked for it.
 */
void
dot1x_send_mka(Dot1xIntf *intf);

/*
 * Create/recreate all SAs known to the principal MKA instance of the
 * real or virtual port using the current SAK. Called by the CP state
 * machine.
 */
void
dot1x_create_sas(Dot1xPort *port);

/*
 * Delete all SAs of the real or virtual port. Called by the CP state
 * machine.
 */
void
dot1x_delete_sas(Dot1xPort *port);

/*
 * Enable all receive SAs known to the principal MKA instance of the
 * real or virtual port. Called by the CP state machine.
 */
void
dot1x_enable_receive_sas(Dot1xPort *port);

/*
 * Enable the transmit SA of the real or virtual port. Called by the
 * CP state machine.
 */
void
dot1x_enable_transmit_sa(Dot1xPort *port);

/*
 * Process a received MAC frame containing a MKA PDU.
 */
void
dot1x_receive_mkpdu(
  Dot1xIntf *intf,
  const MacsecAddress *dst, const MacsecAddress *src,
  const unsigned char *msdu_buf, unsigned int msdu_len);

/*
 * Get pointer to the current key server of a port, or NULL if there
 * is no key server.
 */
Dot1xMkaKeySource *
dot1x_get_key_server(const Dot1xPort *port);

/*
 * Run test vectors through KEK, ICK and SAK derivation
 * functions. Return 1 if successful, 0 otherwise.
 */
int
dot1x_test_mka(void);

#endif /* _DOT1XPAE_MKA_H */
