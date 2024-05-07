/**

   IEEE 802.1X-2010 network announcement PDU encode/decode routines.

   File: dot1xpae_anpdu.c

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#include "dot1xpae_anpdu.h"
#include "dot1xpae_util.h"
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#ifdef DOT1X_ENCODE_FAIL
#include "encodefail.h"
#else
#define ENCODE_FAIL(x, y)
#endif

/*
 * IEEE Std 802.1X-2010 11.12, Table 11-8
 */
#define TYPE_ACCESS_INFO 111
#define TYPE_CIPHER_SUITES 112
#define TYPE_KMD 113
#define TYPE_NID_SET 114
#define TYPE_ORG_SET 126
#define TYPE_ORG 127

typedef struct {
  unsigned char *buf;
  unsigned len;
  unsigned pos;
  Dot1xIntf *intf;
} WriteBuffer;

typedef struct {
  const unsigned char *buf;
  unsigned len;
  unsigned pos;
  Dot1xIntf *intf;
} ReadBuffer;

static int
encode_tlv_header(WriteBuffer *b, unsigned type, unsigned length);
static int
decode_tlv_header(ReadBuffer *b, unsigned *type, unsigned *length);
static int
encode_access_info(WriteBuffer *b, const Dot1xAccessInformation *ai);
static int
decode_access_info(ReadBuffer *b, Dot1xAccessInformation *ai);
static int
encode_cipher_suites(
  WriteBuffer *b, const MacsecCipherCapability *ccv, unsigned len);
static int
decode_cipher_suites(
  ReadBuffer *b, MacsecCipherCapability *ccv, unsigned len);
static int
encode_uint16(WriteBuffer *b, unsigned n);
static int
decode_uint16(ReadBuffer *b, unsigned *n);
static int
encode_uint8(WriteBuffer *b, unsigned n);
static int
decode_uint8(ReadBuffer *b, unsigned *n);
static int
encode_buffer(WriteBuffer *b, const unsigned char *buf, unsigned len);
static int
decode_buffer(ReadBuffer *b, unsigned char *buf, unsigned len);
static int
decode_any(ReadBuffer *b, unsigned len);
static void
encode_error(WriteBuffer *b, const char *fmt, ...);
static void
decode_error(ReadBuffer *b, const char *fmt, ...);

static void
print_network(Dot1xIntf *intf, unsigned indent, const Dot1xAnpduNetwork *n);
static void
print_access_information(
  Dot1xIntf *intf, unsigned indent, const Dot1xAccessInformation *ai);

static void
print(Dot1xIntf *intf, unsigned indent, const char *fmt, ...);

/*
 * Public functions.
 */

int
dot1x_encode_announcement(
  Dot1xIntf *intf, const Dot1xAnpdu *pdu,
  unsigned char *buf, unsigned max, unsigned *len)
{
  WriteBuffer buffer = {buf, max, 0, intf}, *b = &buffer;
  const Dot1xAnpduNetwork *n;
  unsigned network_num = 0, length;
  int i;

  /* encode default network followed by NID TLV sets */
  n = &pdu->default_network;
  while (1)
    {
      if (n->has_access_information)
        {
          if (!encode_tlv_header(b, TYPE_ACCESS_INFO, 2) ||
              !encode_access_info(b, &n->access_information))
            return 0;
        }

      if (n->has_cipher_suites)
        {
          length = 0;
          for (i = 0; i < MACSEC_CIPHER_SUITE_COUNT; i++)
            if (n->cipher_suites[i].implemented)
              length += 10;
          if (!encode_tlv_header(b, TYPE_CIPHER_SUITES, length) ||
              !encode_cipher_suites(b, n->cipher_suites, length))
            return 0;
        }

      if (n->has_kmd)
        {
          if (!encode_tlv_header(b, TYPE_KMD, n->kmd.len) ||
              !encode_buffer(b, n->kmd.buf, n->kmd.len))
            return 0;
        }

      /* finish if no more NID TLV sets */
      if (network_num >= pdu->network_num)
        break;

      n = &pdu->network_tab[network_num++];

      /* begin encoding NID TLV set */
      if (!encode_tlv_header(b, TYPE_NID_SET, n->nid.len) ||
          !encode_buffer(b, n->nid.buf, n->nid.len))
        return 0;
    }

  *len = b->pos;
  return 1;
}

int
dot1x_decode_announcement(
  Dot1xIntf *intf, Dot1xAnpdu *pdu,
  const unsigned char *buf, unsigned len)
{
  ReadBuffer buffer = {buf, len, 0, intf}, *b = &buffer;
  unsigned type, length;
  Dot1xAnpduNetwork *n = &pdu->default_network;

  memset(pdu, 0, sizeof *pdu);

  while (b->pos < b->len)
    {
      /* get TLV type and length */
      if (!decode_tlv_header(b, &type, &length))
        return 0;

      switch (type)
        {
        case TYPE_NID_SET:
          if (pdu->network_num >= DOT1X_MAX_NETWORKS)
            {
              decode_error(b, "too many networks, ignoring one");
              if (!decode_any(b, length))
                return 0;
              n = NULL;
              break;
            }
          n = &pdu->network_tab[pdu->network_num++];
          n->nid.len = length;
          if (!decode_buffer(b, n->nid.buf, n->nid.len))
            return 0;
          break;

        case TYPE_ACCESS_INFO:
          if (!n)
            goto ignore;
          if (!decode_access_info(b, &n->access_information))
            return 0;
          n->has_access_information = 1;
          break;

        case TYPE_CIPHER_SUITES:
          if (length % 10 != 0)
            {
              decode_error(
                b, "cipher suites not multiple of 10 octets, ignoring");
              goto ignore;
            }
          if (!n)
            goto ignore;
          if (!decode_cipher_suites(b, n->cipher_suites, length))
            return 0;
          n->has_cipher_suites = 1;
          break;

        case TYPE_KMD:
          if (!n)
            goto ignore;
          n->kmd.len = length;
          if (!decode_buffer(b, n->kmd.buf, n->kmd.len))
            return 0;
          n->has_kmd = 1;
          break;

        case TYPE_ORG_SET:
        case TYPE_ORG:
        default:
          decode_error(b, "ignoring TLV type %u", type);
        ignore:
          if (!decode_any(b, length))
            return 0;
          break;
        }
    }

  return 1;
}

int
dot1x_encode_announcement_req(
  Dot1xIntf *intf, const Dot1xAnpduReq *req,
  unsigned char *buf, unsigned max, unsigned *len)
{
  WriteBuffer buffer = {buf, max, 0, intf}, *b = &buffer;
  const Dot1xAnpduReqNetwork *n;
  int i;

  for (i = 0; i < req->network_num; i++)
    {
      n = &req->network_tab[i];

      if (!encode_tlv_header(b, TYPE_NID_SET, n->nid.len) ||
          !encode_buffer(b, n->nid.buf, n->nid.len))
        return 0;

      if (n->has_access_information)
        {
          if (!encode_tlv_header(b, TYPE_ACCESS_INFO, 2) ||
              !encode_access_info(b, &n->access_information))
            return 0;
        }
    }

  *len = b->pos;
  return 1;
}

int
dot1x_decode_announcement_req(
  Dot1xIntf *intf, Dot1xAnpduReq *req,
  const unsigned char *buf, unsigned len)
{
  ReadBuffer buffer = {buf, len, 0, intf}, *b = &buffer;
  unsigned type, length;
  Dot1xAnpduReqNetwork *n = NULL;

  memset(req, 0, sizeof *req);

  while (b->pos < b->len)
    {
      /* get TLV type and length */
      if (!decode_tlv_header(b, &type, &length))
        return 0;

      switch (type)
        {
        case TYPE_NID_SET:
          if (req->network_num >= DOT1X_MAX_NETWORKS)
            {
              decode_error(b, "too many networks, ignoring one");
              if (!decode_any(b, length))
                return 0;
              n = NULL;
              break;
            }
          n = &req->network_tab[req->network_num++];
          n->nid.len = length;
          if (!decode_buffer(b, n->nid.buf, n->nid.len))
            return 0;
          break;

        case TYPE_ACCESS_INFO:
          if (!n)
            goto ignore;
          if (!decode_access_info(b, &n->access_information))
            return 0;
          n->has_access_information = 1;
          break;

        default:
          decode_error(b, "ignoring TLV type %u", type);
        ignore:
          if (!decode_any(b, length))
            return 0;
          break;
        }
    }

  return 1;
}

void
dot1x_print_announcement(Dot1xIntf *intf, const Dot1xAnpdu *pdu)
{
  const Dot1xAnpduNetwork *n;
  Dot1xFormatBuffer fb;
  int i;

  print(intf, 1, "default network");
  print_network(intf, 2, &pdu->default_network);

  for (i = 0; i < pdu->network_num; i++)
    {
      print(intf, 1, "named network #%d", i);
      n = &pdu->network_tab[i];
      print(intf, 2, "NID: %s", dot1x_format_nid(&fb, &n->nid));
      print_network(intf, 2, &pdu->network_tab[i]);
    }
}

void
dot1x_print_announcement_req(Dot1xIntf *intf, const Dot1xAnpduReq *req)
{
  const Dot1xAnpduReqNetwork *n;
  Dot1xFormatBuffer fb;
  int i;

  for (i = 0; i < req->network_num; i++)
    {
      print(intf, 1, "network #%d", i);
      n = &req->network_tab[i];
      print(intf, 2, "NID: %s", dot1x_format_nid(&fb, &n->nid));
      if (n->has_access_information)
        print_access_information(intf, 2, &n->access_information);
    }
}

/*
 * Static functions.
 */

static int
encode_tlv_header(WriteBuffer *b, unsigned type, unsigned length)
{
  unsigned oct1, oct2;

  if (b->pos + 2 + length > b->len)
    {
      encode_error(b, "TLV too large to encode");
      return 0;
    }

  oct1 = type << 1;
  oct1 |= length >> 8 & 0x01;
  oct2 = length & 0xff;

  return
    encode_uint8(b, oct1) &&
    encode_uint8(b, oct2);
}

static int
decode_tlv_header(ReadBuffer *b, unsigned *type, unsigned *length)
{
  unsigned oct1, oct2, t, l;

  if (!decode_uint8(b, &oct1) ||
      !decode_uint8(b, &oct2))
    return 0;

  t = oct1 >> 1;
  l = oct1 << 8 & 0x0100;
  l |= oct2;

  if (b->pos + l > b->len)
    {
      decode_error(b, "truncated TLV");
      return 0;
    }

  if (l > 511)
    {
      decode_error(b, "TLV information string too long");
      return 0;
    }

  *type = t;
  *length = l;
  return 1;
}

static int
encode_access_info(WriteBuffer *b, const Dot1xAccessInformation *ai)
{
  unsigned oct1 = 0;
  unsigned oct2 = 0;

  oct1 = ai->access_status;
  oct1 |= ai->access_requested << 2;
  oct1 |= ai->unauthenticated_access << 3;
  oct1 |= ai->virtual_port_access << 5;
  oct1 |= ai->group_access << 6;

  oct2 = ai->access_capabilities.eap;
  oct2 |= ai->access_capabilities.eap_mka << 1;
  oct2 |= ai->access_capabilities.eap_mka_macsec << 2;
  oct2 |= ai->access_capabilities.mka << 3;
  oct2 |= ai->access_capabilities.mka_macsec << 4;
  oct2 |= ai->access_capabilities.higher_layer << 5;
  oct2 |= ai->access_capabilities.higher_layer_fallback << 6;
  oct2 |= ai->access_capabilities.vendor_specific << 7;

  return
    encode_uint8(b, oct1) &&
    encode_uint8(b, oct2);
}

static int
decode_access_info(ReadBuffer *b, Dot1xAccessInformation *ai)
{
  unsigned oct1, oct2;

  if (!decode_uint8(b, &oct1) ||
      !decode_uint8(b, &oct2))
    return 0;

  ai->access_status = oct1 & 0x03;
  ai->access_requested = oct1 >> 2 & 0x01;
  ai->unauthenticated_access = oct1 >> 3 & 0x03;
  ai->virtual_port_access = oct1 >> 5 & 0x01;
  ai->group_access = oct1 >> 6 & 0x01;

  ai->access_capabilities.eap = oct2 & 0x01;
  ai->access_capabilities.eap_mka = oct2 >> 1 & 0x01;
  ai->access_capabilities.eap_mka_macsec = oct2 >> 2 & 0x01;
  ai->access_capabilities.mka = oct2 >> 3 & 0x01;
  ai->access_capabilities.mka_macsec = oct2 >> 4 & 0x01;
  ai->access_capabilities.higher_layer = oct2 >> 5 & 0x01;
  ai->access_capabilities.higher_layer_fallback = oct2 >> 6 & 0x01;
  ai->access_capabilities.vendor_specific = oct2 >> 7 & 0x01;

  return 1;
}

static int
encode_cipher_suites(
  WriteBuffer *b, const MacsecCipherCapability *ccv, unsigned length)
{
  const unsigned char (*refnr)[8];
  unsigned l = length;
  int i;

  for (i = 0; i < MACSEC_CIPHER_SUITE_COUNT; i++)
    {
      if (l < 10 || !ccv[i].implemented)
        continue;
      if (!dot1x_get_cipher_suite_refnr(&refnr, i))
        goto fail;
      if (!encode_uint16(b, ccv[i].capability) ||
          !encode_buffer(b, *refnr, sizeof *refnr))
        return 0;
      l -= 10;
    }
  if (l)
    goto fail;
  return 1;

 fail:
  encode_error(b, "cannot encode MACsec cipher suite");
  return 0;
}

static int
decode_cipher_suites(
  ReadBuffer *b, MacsecCipherCapability *ccv, unsigned length)
{
  unsigned char refnr[8];
  MacsecCipherSuite cs;
  unsigned l = length;
  unsigned mc;

  while (l >= 10)
    {
      if (!decode_uint16(b, &mc) ||
          !decode_buffer(b, refnr, 8))
        return 0;
      if (!dot1x_get_cipher_suite_ident(&cs, &refnr))
        goto fail;

      ccv[cs].implemented = 1;
      ccv[cs].capability = mc;
      l -= 10;
    }
  if (l)
    goto fail;
  return 1;

 fail:
  decode_error(b, "cannot decode MACsec cipher suites");
  return 0;

}

static int
encode_uint16(WriteBuffer *b, unsigned n)
{
  unsigned oct1, oct0;

  oct1 = n >> 8 & 0xff;
  oct0 = n & 0xff;

  return
    encode_uint8(b, oct1) &&
    encode_uint8(b, oct0);
}

static int
decode_uint16(ReadBuffer *b, unsigned *n)
{
  unsigned oct1, oct0;

  if (!(decode_uint8(b, &oct1) &&
        decode_uint8(b, &oct0)))
    return 0;

  *n = oct1 << 8;
  *n |= oct0;
  return 1;
}

static int
encode_uint8(WriteBuffer *b, unsigned n)
{
  if (b->pos >= b->len)
    {
      encode_error(b, "out of output space");
      return 0;
    }

  b->buf[b->pos] = (unsigned char)n;
  ENCODE_FAIL(&b->buf[b->pos], 1);
  b->pos++;
  return 1;
}

static int
decode_uint8(ReadBuffer *b, unsigned *n)
{
  if (b->pos >= b->len)
    {
      decode_error(b, "out of input data");
      return 0;
    }

  *n = (unsigned)b->buf[b->pos++];
  return 1;
}

static int
encode_buffer(WriteBuffer *b, const unsigned char *buf,
              unsigned len)
{
  if (b->pos + len > b->len)
    {
      encode_error(b, "out of output space");
      return 0;
    }

  memcpy(b->buf + b->pos, buf, len);
  ENCODE_FAIL(b->buf, len);
  b->pos += len;
  return 1;
}

static int
decode_buffer(ReadBuffer *b, unsigned char *buf, unsigned len)
{
  if (b->pos + len > b->len)
    {
      decode_error(b, "out of input data");
      return 0;
    }

  memcpy(buf, b->buf + b->pos, len);
  b->pos += len;
  return 1;
}

static int
decode_any(ReadBuffer *b, unsigned len)
{
  if (b->pos + len > b->len)
    {
      decode_error(b, "out of input data");
      return 0;
    }

  b->pos += len;
  return 1;
}

static void
encode_error(WriteBuffer *b, const char *fmt, ...)
{
  va_list ap;
  Dot1xFormatBuffer fb;

  va_start(ap, fmt);
  dot1x_error(
    b->intf, "announcement encode: %s",
    dot1x_format_vprintf(&fb, fmt, ap));
  va_end(ap);
}

static void
decode_error(ReadBuffer *b, const char *fmt, ...)
{
  va_list ap;
  Dot1xFormatBuffer fb;

  va_start(ap, fmt);
  dot1x_error(
    b->intf, "announcement decode: %s",
    dot1x_format_vprintf(&fb, fmt, ap));
  va_end(ap);
}

static void
print_network(Dot1xIntf *f, unsigned t, const Dot1xAnpduNetwork *n)
{
  Dot1xFormatBuffer fb;
  int i;

  if (n->has_access_information)
    print_access_information(f, t, &n->access_information);

  if (n->has_cipher_suites)
    {
      print(f, t, "MACsec cipher suites");
      for (i = 0; i < MACSEC_CIPHER_SUITE_COUNT; i++)
        {
          if (!n->cipher_suites[i].implemented)
            continue;
          print(f, t + 1, "%s", dot1x_cipher_suite(i));
          print(
            f, t + 2, "MACsec capability: %s",
            dot1x_macsec_capability(n->cipher_suites[i].capability));
        }
    }

  if (n->has_kmd)
    print(f, t, "KMD: %s", dot1x_format_kmd(&fb, &n->kmd));
}

static void
print_access_information(
  Dot1xIntf *f, unsigned t, const Dot1xAccessInformation *ai)
{
  print(f, t, "Access information");
  print(f, t + 1, "Access status: %s", dot1x_access_status(ai->access_status));
  print(f, t + 1, "Access requested: %u", ai->access_requested);
  print(
    f, t + 1, "Unauthenticated access: %s",
    dot1x_unauthenticated_access(ai->unauthenticated_access));
  print(f, t + 1, "Virtual port access: %u", ai->virtual_port_access);
  print(f, t + 1, "Group access: %u", ai->group_access);

  print(f, t, "Access capabilities");

  print(f, t + 1, "EAP: %u", ai->access_capabilities.eap);
  print(f, t + 1, "EAP + MKA: %u", ai->access_capabilities.eap_mka);
  print(
    f, t + 1, "EAP + MKA + MACsec: %u",
    ai->access_capabilities.eap_mka_macsec);
  print(f, t + 1, "MKA: %u", ai->access_capabilities.mka);
  print(f, t + 1, "MKA + MACsec: %u", ai->access_capabilities.mka_macsec);
  print(f, t + 1, "Higher layer: %u", ai->access_capabilities.higher_layer);
  print(
    f, t + 1, "Higher layer fallback: %u",
    ai->access_capabilities.higher_layer_fallback);
  print(
    f, t + 1, "Vendor specific: %u", ai->access_capabilities.vendor_specific);
}

static void
print(Dot1xIntf *intf, unsigned indent, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  dot1x_printf(intf, "%*s", 4 * indent, "");
  dot1x_vprintf(intf, fmt, ap);
  dot1x_printf(intf, "\n");
  va_end(ap);
}
