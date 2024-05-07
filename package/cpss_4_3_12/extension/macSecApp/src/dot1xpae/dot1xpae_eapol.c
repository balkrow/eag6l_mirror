/**

   IEEE 802.1X-2010 EAPOL transmit/receive.

   File: dot1xpae_eapol.c

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#include "dot1xpae_eapol.h"

#ifdef DOT1X_ENCODE_FAIL
#include "encodefail.h"
#else
#define ENCODE_FAIL(x, y)
#endif

static const unsigned char ethertype[2] = {0x88, 0x8e};

/*
 * Public functions.
 */

void
dot1x_encode_eapol_header(
  unsigned char *msdu_buf,
  unsigned version,
  unsigned type,
  unsigned length)
{
  int pos = 0;

  msdu_buf[pos++] = ethertype[0];
  msdu_buf[pos++] = ethertype[1];
  msdu_buf[pos++] = (unsigned char)version;
  msdu_buf[pos++] = (unsigned char)type;
  msdu_buf[pos++] = (length >> 8);
  msdu_buf[pos++] = (length & 0xff);
  ENCODE_FAIL(msdu_buf, pos);
}

int
dot1x_decode_eapol_header(
  const unsigned char *msdu_buf,
  unsigned *version,
  unsigned *type,
  unsigned *length)
{
  int pos = 0;

  if (msdu_buf[pos++] != ethertype[0])
    return -1;
  if (msdu_buf[pos++] != ethertype[1])
    return -1;
  *version = msdu_buf[pos++];
  *type = msdu_buf[pos++];
  *length = msdu_buf[pos++] << 8;
  *length |= msdu_buf[pos++];
  return 0;
}
