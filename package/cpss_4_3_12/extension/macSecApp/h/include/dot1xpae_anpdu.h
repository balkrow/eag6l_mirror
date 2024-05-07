/**

   IEEE 802.1X-2010 network announcement PDU encode/decode routines.

   File: dot1xpae_anpdu.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_ANPDU_H
#define _DOT1XPAE_ANPDU_H

#include "dot1xpae_types.h"

/*
 * Encode the PDU structure pointed to by `pdu' into buffer `buf' the
 * size of which is `max' and return the number of octets written in
 * the location pointed to by `len'. Return nonzero if successful,
 * zero if the PDU could not be encoded.
 */
int
dot1x_encode_announcement(
  Dot1xIntf *intf, const Dot1xAnpdu *pdu,
  unsigned char *buf, unsigned max, unsigned *len);

/*
 * Decode the buffer `buf' the size of which is `len' into the PDU
 * structure pointed to by `pdu'. Return nonzero if successful, zero
 * if the PDU could not be decoded.
 */
int
dot1x_decode_announcement(
  Dot1xIntf *intf, Dot1xAnpdu *pdu,
  const unsigned char *buf, unsigned len);

/*
 * Encode the announcement request structure pointed to by `req' into
 * buffer `buf' the size of which is `max' and return the number of
 * octets written in the location pointed to by `len'. Return nonzero
 * if successful, zero if the request could not be encoded.
 */
int
dot1x_encode_announcement_req(
  Dot1xIntf *intf, const Dot1xAnpduReq *req,
  unsigned char *buf, unsigned max, unsigned *len);

/*
 * Decode the buffer `buf' the size of which is `len' into the
 * announcement request structure pointed to by `req'. Return nonzero
 * if successful, zero if the request could not be decoded.
 */
int
dot1x_decode_announcement_req(
  Dot1xIntf *intf, Dot1xAnpduReq *req,
  const unsigned char *buf, unsigned len);

/*
 * Pretty-print PDU contents.
 */
void
dot1x_print_announcement(Dot1xIntf *intf, const Dot1xAnpdu *pdu);

/*
 * Pretty-print announcement request contents.
 */
void
dot1x_print_announcement_req(Dot1xIntf *intf, const Dot1xAnpduReq *req);

#endif /* _DOT1XPAE_ANPDU_H */
