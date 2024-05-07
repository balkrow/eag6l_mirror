/**

   IEEE 802.1X-2010 EAPOL transmit/receive.

   File: dot1xpae_eapol.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_EAPOL_H
#define _DOT1XPAE_EAPOL_H

/*
 * Size of an EAPOL header, including ethertype.
 */
#define DOT1X_EAPOL_HEADER_SIZE 6

/*
 * EAPOL packet types.
 */
#define DOT1X_EAPOL_EAP 0
#define DOT1X_EAPOL_START 1
#define DOT1X_EAPOL_LOGOFF 2
#define DOT1X_EAPOL_KEY 3
#define DOT1X_EAPOL_ASF 4
#define DOT1X_EAPOL_MKA 5
#define DOT1X_EAPOL_ANNOUNCEMENT_GEN 6
#define DOT1X_EAPOL_ANNOUNCEMENT_SPC 7
#define DOT1X_EAPOL_ANNOUNCEMENT_REQ 8

/*
 * Write a 2-octet ethertype and a 4-octet EAPOL header with the
 * specified EAPOL version, packet type and body length in an MSDU
 * buffer.
 */
void
dot1x_encode_eapol_header(
  unsigned char *msdu_buf,
  unsigned version,
  unsigned type,
  unsigned length);

/*
 * Read a 2-octet ethertype and a 4-octet EAPOL header from an MSDU
 * buffer and store the EAPOL version, packet type and body length in
 * the specified locations. Return zero if successful, or -1 if the
 * ethertype was not EAPOL.
 */
int
dot1x_decode_eapol_header(
  const unsigned char *msdu_buf,
  unsigned *version,
  unsigned *type,
  unsigned *length);

#endif /* _DOT1XPAE_EAPOL_H */
