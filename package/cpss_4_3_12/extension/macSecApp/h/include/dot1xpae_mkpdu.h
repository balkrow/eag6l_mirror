/**

   MKA PDU encode/decode routines.

   File: dot1xpae_mka_pdu.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_MKA_PDU_H
#define _DOT1XPAE_MKA_PDU_H

#include "dot1xpae_types.h"
#include "dot1xpae_crypto.h"
#include <stdint.h>

/*
 * Encode the MKA PDU structure pointed to by `pdu' into buffer `buf'
 * the size of which is `max' and return the number of octets written
 * in the location pointed to by `len'. Return nonzero if successful,
 * zero if the PDU could not be encoded.
 */
int
dot1x_encode_mkpdu(
  Dot1xIntf *intf, const Dot1xMkpdu *pdu,
  unsigned char *buf, unsigned max, unsigned *len);

/*
 * Decode the buffer `buf' the size of which is `len' into the MKA PDU
 * structure pointed to by `pdu'. Return nonzern if successful, 0 if
 * the PDU could not be decoded.
 */
int
dot1x_decode_mkpdu(
  Dot1xIntf *intf, Dot1xMkpdu *pdu,
  const unsigned char *buf, unsigned len);

/*
 * Pretty-print PDU contents.
 */
void
dot1x_print_mkpdu(Dot1xIntf *intf, Dot1xMkpdu *pdu);

#endif /* _DOT1XPAE_MKA_PDU_H */
