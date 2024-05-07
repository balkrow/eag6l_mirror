/**

   MKA PDU encode/decode routines.

   File: dot1xpae_mka_pdu.c

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/
#include "dot1xpae_mkpdu.h"
#include "dot1xpae_anpdu.h"
#include "dot1xpae_util.h"
#include <stdint.h>
#include <string.h>

#ifdef DOT1X_ENCODE_FAIL
#include "encodefail.h"
#else
#define ENCODE_FAIL(x, y)
#endif

/*
 * Parameter set type codes.
 */
#define PSET_LIVE_PEER_LIST 1
#define PSET_POTENTIAL_PEER_LIST 2
#define PSET_SAK_USE 3
#define PSET_DISTRIBUTED_SAK 4
#define PSET_DISTRIBUTED_CAK 5
#define PSET_KMD 6
#define PSET_ANNOUNCEMENT 7
#define PSET_XPN 8
#define PSET_ICV_INDICATOR 255

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

typedef unsigned char TypeMap[32];

static const unsigned char mka_algorithm_8021x_2009[4] = {
  0x00, 0x80, 0xc2, 0x01};

static int
get_type(TypeMap *map, unsigned type);
static void
set_type(TypeMap *map, unsigned type);

static int
encode_basic(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_basic(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_sak_use(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_sak_use(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_distributed_sak(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_distributed_sak(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_distributed_cak(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_distributed_cak(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_kmd(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_kmd(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_announcement(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_announcement(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_live_peer_list(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_live_peer_list(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_potential_peer_list(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_potential_peer_list(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_icv_indicator(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_icv_indicator(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
decode_icv_plain(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
decode_ignored(ReadBuffer *b, Dot1xMkpdu *pdu);
static int
encode_xpn(WriteBuffer *b, const Dot1xMkpdu *pdu);
static int
decode_xpn(ReadBuffer *b, Dot1xMkpdu *pdu);

static int
encode_header(
  WriteBuffer *b, uint32_t type, uint32_t flags, uint32_t length);
static int
decode_header(
  ReadBuffer *b, uint32_t *type, uint32_t *flags, uint32_t *length);
static int
encode_peer_list(WriteBuffer *b, const Dot1xMkpduPeerList *pl);
static int
decode_peer_list(ReadBuffer *b, Dot1xMkpduPeerList *pl);
static int
encode_mka_algorithm(WriteBuffer *b, const Dot1xMkaAlgorithm ma);
static int
decode_mka_algorithm(ReadBuffer *b, Dot1xMkaAlgorithm *ma);
static int
encode_cipher_suite(WriteBuffer *b, MacsecCipherSuite cs);
static int
decode_cipher_suite(ReadBuffer *b, MacsecCipherSuite *cs);
static int
encode_sci(WriteBuffer *b, const MacsecSci *sci);
static int
decode_sci(ReadBuffer *b, MacsecSci *sci);
static int
encode_key_identifier(WriteBuffer *b, const Dot1xKeyIdentifier *ki);
static int
decode_key_identifier(ReadBuffer *b, Dot1xKeyIdentifier *ki);
static int
encode_member_identifier(WriteBuffer *b, const Dot1xMemberIdentifier *mi);
static int
decode_member_identifier(ReadBuffer *b, Dot1xMemberIdentifier *mi);
static int
encode_ckn(WriteBuffer *b, const Dot1xCkn *ckn);
static int
decode_ckn(ReadBuffer *b, Dot1xCkn *ckn);
static int
encode_cak_wrap(WriteBuffer *b, const Dot1xCryptoWrap *ckw);
static int
decode_cak_wrap(ReadBuffer *b, Dot1xCryptoWrap *ckw);
static int
encode_sak_wrap(WriteBuffer *b, const Dot1xCryptoWrap *skw);
static int
decode_sak_wrap(ReadBuffer *b, Dot1xCryptoWrap *skw);
static int
encode_icv(WriteBuffer *b, const Dot1xIcv *icv);
static int
decode_icv(ReadBuffer *b, Dot1xIcv *icv);
static int
encode_uint32(WriteBuffer *b, uint32_t n);
static int
decode_uint32(ReadBuffer *b, uint32_t *n);
static int
encode_uint16(WriteBuffer *b, unsigned n);
static int
decode_uint16(ReadBuffer *b, unsigned *n);
static int
encode_uint8(WriteBuffer *b, unsigned n);
static int
decode_uint8(ReadBuffer *b, unsigned *n);
static int
encode_padding(WriteBuffer *b);
static int
decode_padding(ReadBuffer *b);
static int
encode_buffer(WriteBuffer *b, const unsigned char *buf, unsigned len);
static int
decode_buffer(ReadBuffer *b, unsigned char *buf, unsigned len);
static int
encode_null(WriteBuffer *b, unsigned len);
static int
decode_any(ReadBuffer *b, unsigned len);
static void
encode_error(WriteBuffer *b, const char *fmt, ...);
static void
decode_error(ReadBuffer *b, const char *fmt, ...);

int
dot1x_encode_mkpdu(
  Dot1xIntf *intf, const Dot1xMkpdu *pdu,
  unsigned char *buf, unsigned max, unsigned *len)
{
  WriteBuffer buffer = {buf, max, 0, intf}, *b = &buffer;

  if (!encode_basic(b, pdu))
    return 0;

  if ((pdu->has_sak_use && !encode_sak_use(b, pdu)) ||
      (pdu->has_distributed_sak && !encode_distributed_sak(b, pdu)) ||
      (pdu->has_distributed_cak && !encode_distributed_cak(b, pdu)) ||
      (pdu->has_kmd && !encode_kmd(b, pdu)) ||
      (pdu->has_announcement && !encode_announcement(b, pdu)) ||
      (pdu->has_xpn && !encode_xpn(b, pdu)))
    return 0;

  if (!encode_live_peer_list(b, pdu) ||
      !encode_potential_peer_list(b, pdu))
    return 0;

  if (!encode_icv_indicator(b, pdu))
    return 0;

  *len = b->pos;
  return 1;
}

int
dot1x_decode_mkpdu(
  Dot1xIntf *intf, Dot1xMkpdu *pdu,
  const unsigned char *buf, unsigned len)
{
  ReadBuffer buffer = {buf, len, 0, intf}, *b = &buffer;
  TypeMap map = {0}, *m = &map;
  Dot1xAnpdu *ann;
  unsigned type;

  ann = pdu->announcement;
  memset(pdu, 0, sizeof *pdu);
  pdu->announcement = ann;

  if (b->pos >= b->len)
    {
      decode_error(b, "empty PDU");
      return 0;
    }

  pdu->version = b->buf[b->pos];

  if (!decode_basic(b, pdu))
    return 0;

  while (b->pos < b->len)
    {
      /* if 16 bytes remain assume it is the ICV without ICV indicator */
      if (b->pos + 16 == b->len)
        {
          if (!decode_icv_plain(b, pdu))
            return 0;
          set_type(m, PSET_ICV_INDICATOR);
          break;
        }

      type = b->buf[b->pos];

      /* ignore other than the first occurrence of a parameter set */
      if (get_type(m, type))
        {
          if (!decode_ignored(b, pdu))
            return 0;
          decode_error(b, "ignoring repeated parameter set %u", type);
          continue;
        }

      /* ignore all parameter sets after the ICV */
      if (get_type(m, PSET_ICV_INDICATOR))
        {
          if (!decode_ignored(b, pdu))
            return 0;
          decode_error(b, "ignoring post-ICV parameter set %u", type);
          continue;
        }

      set_type(m, type);

      switch (type)
        {
        case 0:
          /* 0 can only occur as the MKA version in the basic set */
          decode_error(b, "bad parameter set type 0");
          return 0;
        case PSET_SAK_USE:
          if (!decode_sak_use(b, pdu))
            return 0;
          pdu->has_sak_use = 1;
          break;
        case PSET_DISTRIBUTED_SAK:
          if (!decode_distributed_sak(b, pdu))
            return 0;
          pdu->has_distributed_sak = 1;
          break;
        case PSET_DISTRIBUTED_CAK:
          if (!decode_distributed_cak(b, pdu))
            return 0;
          pdu->has_distributed_cak = 1;
          break;
        case PSET_KMD:
          if (!decode_kmd(b, pdu))
            return 0;
          pdu->has_kmd = 1;
          break;
        case PSET_ANNOUNCEMENT:
          if (!decode_announcement(b, pdu))
            return 0;
          pdu->has_announcement = 1;
          break;
        case PSET_LIVE_PEER_LIST:
          if (!decode_live_peer_list(b, pdu))
            return 0;
          break;
        case PSET_POTENTIAL_PEER_LIST:
          if (!decode_potential_peer_list(b, pdu))
            return 0;
          break;
        case PSET_ICV_INDICATOR:
          if (!decode_icv_indicator(b, pdu))
            return 0;
          break;
        case PSET_XPN:
          if (!decode_xpn(b, pdu))
            return 0;
          pdu->has_xpn = 1;
          break;
        default:
          decode_error(b, "ignoring parameter set type %u", type);
          if (!decode_ignored(b, pdu))
            return 0;
          break;
        }
    }

  if (!get_type(m, PSET_ICV_INDICATOR))
    {
      decode_error(b, "no ICV");
      return 0;
    }

  /* assume empty peer lists if omitted from the PDU */
  if (!get_type(m, PSET_LIVE_PEER_LIST))
    pdu->lpl.num = 0;
  if (!get_type(m, PSET_POTENTIAL_PEER_LIST))
    pdu->ppl.num = 0;

  return 1;
}

void
dot1x_print_mkpdu(Dot1xIntf *intf, Dot1xMkpdu *pdu)
{
  Dot1xFormatBuffer fb;
  const Dot1xMkpduPeer *p;
  int i;

  dot1x_printf(
    intf, "  Basic parameter set:\n");
  dot1x_printf(
    intf, "    MKA version: %u\n", pdu->version);
  dot1x_printf(
    intf, "    Key server priority: %lu\n",
    (unsigned long)pdu->key_server_priority);
  dot1x_printf(
    intf, "    Key server: %u\n", pdu->key_server);
  dot1x_printf(
    intf, "    MACsec desired: %u\n", pdu->macsec_desired);
  dot1x_printf(
    intf, "    MACsec capability: %s\n",
    dot1x_macsec_capability(pdu->macsec_capability));
  dot1x_printf(
    intf, "    SCI: %s\n", dot1x_format_sci(&fb, &pdu->sci));
  dot1x_printf(
    intf, "    MI: %s\n", dot1x_format_member_identifier(&fb, &pdu->mi));
  dot1x_printf(
    intf, "    MN: %lu\n", (unsigned long)pdu->mn);
  dot1x_printf(
    intf, "    Algorithm agility: %s\n",
    dot1x_mka_algorithm(pdu->mka_algorithm));
  dot1x_printf(
    intf, "    CKN: %s\n", dot1x_format_ckn(&fb, &pdu->ckn));

  if (pdu->has_sak_use)
    {
      dot1x_printf(
        intf, "  SAK use parameter set:\n");
      dot1x_printf(
        intf, "    Latest key association number: %u\n", pdu->lan);
      dot1x_printf(
        intf, "    Latest key tx: %u\n", pdu->ltx);
      dot1x_printf(
        intf, "    Latest key rx: %u\n", pdu->lrx);
      dot1x_printf(
        intf, "    Old key association number: %u\n", pdu->oan);
      dot1x_printf(
        intf, "    Old key tx: %u\n", pdu->otx);
      dot1x_printf(
        intf, "    Old key rx: %u\n", pdu->orx);
      dot1x_printf(
        intf, "    Plain tx: %u\n", pdu->ptx);
      dot1x_printf(
        intf, "    Plain rx: %u\n", pdu->prx);
      dot1x_printf(
        intf, "    Delay protect: %u\n", pdu->dp);
      dot1x_printf(
        intf, "    Latest key identifier: %s\n",
        dot1x_format_key_identifier(&fb, &pdu->lki));
      dot1x_printf(
        intf, "    Latest key lowest acceptable packet number: %llu\n",
        (unsigned long long)pdu->llpn);
      dot1x_printf(
        intf, "    Old key identifier: %s\n",
        dot1x_format_key_identifier(&fb, &pdu->oki));
      dot1x_printf(
        intf, "    Old key lowest acceptable packet number: %llu\n",
        (unsigned long long)pdu->olpn);
    }

  if (pdu->has_distributed_sak)
    {
      dot1x_printf(
        intf, "  Distributed SAK parameter set:\n");
      dot1x_printf(
        intf, "    Distributed AN: %u\n", pdu->distributed_an);
      dot1x_printf(
        intf, "    Confidentiality offset: %s\n",
        dot1x_confidentiality_offset(pdu->distributed_co));
      dot1x_printf(
        intf, "    Key number: %lu\n", (long unsigned)pdu->distributed_kn);
      dot1x_printf(
        intf, "    MACsec cipher suite: %s\n",
        dot1x_cipher_suite(pdu->distributed_cs));
      dot1x_printf(
        intf, "    AES key wrap of SAK: %s\n",
        dot1x_format_wrap(&fb, &pdu->distributed_sak));
    }

  if (pdu->has_distributed_cak)
    {
      dot1x_printf(
        intf, "  Distributed CAK parameter set:\n");
      dot1x_printf(
        intf, "    AES key wrap of CAK: %s\n",
        dot1x_format_wrap(&fb, &pdu->distributed_cak));
      dot1x_printf(
        intf, "    CAK name: %s\n",
        dot1x_format_ckn(&fb, &pdu->distributed_ckn));
    }

  if (pdu->has_kmd)
    {
      dot1x_printf(intf, "  KMD parameter set:\n");
      dot1x_printf(intf, "    KMD: %s\n", dot1x_format_kmd(&fb, &pdu->kmd));
    }

  if (pdu->has_announcement)
    {
      dot1x_printf(
        intf, "  Announcement parameter set:\n");
      dot1x_print_announcement(intf, pdu->announcement);
    }

  if (pdu->has_xpn)
    {
      dot1x_printf(
        intf, "  XPN parameter set:\n");
      dot1x_printf(
        intf, "    MKA suspension time: %u\n", pdu->xpn_mst);
      dot1x_printf(
        intf, "    Latest key lowest acceptable packet number (32 MSBs): %u\n",
        pdu->xpn_llpn);
      dot1x_printf(
        intf, "    Old key lowest acceptable packet number (32 MSBs): %u\n",
        pdu->xpn_olpn);
    }

  for (i = 0; i < pdu->lpl.num; i++)
    {
      p = &pdu->lpl.tab[i];
      dot1x_printf(
        intf, "  Live peer:\n");
      dot1x_printf(
        intf, "    Member identifier: %s\n",
        dot1x_format_member_identifier(&fb, &p->mi));
      dot1x_printf(
        intf, "    Message number: %lu\n", (long unsigned)p->mn);
    }

  for (i = 0; i < pdu->ppl.num; i++)
    {
      p = &pdu->ppl.tab[i];
      dot1x_printf(
        intf, "  Potential peer:\n");
      dot1x_printf(
        intf, "    Member identifier: %s\n",
        dot1x_format_member_identifier(&fb, &p->mi));
      dot1x_printf(
        intf, "    Message number: %lu\n", (long unsigned)p->mn);
    }

  dot1x_printf(
    intf, "  ICV: %s\n", dot1x_format_icv(&fb, &pdu->icv));
}

static int
get_type(TypeMap *map, unsigned type)
{
  if (type > 255)
    return 0;

  return !!((*map)[type / 8] & 1 << type % 8);
}

static void
set_type(TypeMap *map, unsigned type)
{
  if (type > 255)
    return;

  (*map)[type / 8] |= 1 << type % 8;
}

static int
encode_basic(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  uint32_t flags, length;
  const int fixed_length = 28;

  flags = pdu->key_server_priority << 4;
  flags |= pdu->key_server << 3;
  flags |= pdu->macsec_desired << 2;
  flags |= (pdu->macsec_capability) & 0x3;

  length = fixed_length + pdu->ckn.len;

  if (!encode_header(b, pdu->version, flags, length))
    return 0;

  return
    encode_sci(b, &pdu->sci) &&
    encode_member_identifier(b, &pdu->mi) &&
    encode_uint32(b, pdu->mn) &&
    encode_mka_algorithm(b, pdu->mka_algorithm) &&
    encode_ckn(b, &pdu->ckn) &&
    encode_padding(b);
}

static int
decode_basic(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;
  const int fixed_length = 28;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  if (length < fixed_length)
    {
      decode_error(b, "basic parameter set too small");
      return 0;
    }

  pdu->key_server_priority = flags >> 4;
  pdu->key_server = flags >> 3 & 1;
  pdu->macsec_desired = flags >> 2 & 1;
  pdu->macsec_capability = flags & 3;

  if (length - fixed_length < 1)
    {
      decode_error(b, "CA key name empty");
      return 0;
    }

  if (length - fixed_length > sizeof pdu->ckn.buf)
    {
      decode_error(b, "CA key name too large");
      return 0;
    }

  pdu->ckn.len = length - fixed_length;

  return
    decode_sci(b, &pdu->sci) &&
    decode_member_identifier(b, &pdu->mi) &&
    decode_uint32(b, &pdu->mn) &&
    decode_mka_algorithm(b, &pdu->mka_algorithm) &&
    decode_ckn(b, &pdu->ckn) &&
    decode_padding(b);
}

static int
encode_sak_use(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  uint32_t flags;

  flags = pdu->lan << 10 & 0x0c00;
  flags |= pdu->ltx << 9;
  flags |= pdu->lrx << 8;
  flags |= pdu->oan << 6 & 0xc0;
  flags |= pdu->otx << 5;
  flags |= pdu->orx << 4;
  flags |= pdu->ptx << 3;
  flags |= pdu->prx << 2;
  flags |= pdu->dp;

  if (!encode_header(b, PSET_SAK_USE, flags, 40))
    return 0;

  return
    encode_key_identifier(b, &pdu->lki) &&
    encode_uint32(b, (uint32_t)pdu->llpn) &&
    encode_key_identifier(b, &pdu->oki) &&
    encode_uint32(b, (uint32_t)pdu->olpn) &&
    encode_padding(b);
}

static int
decode_sak_use(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length, llpnlo, olpnlo;
  const int fixed_length = 40;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  pdu->lan = flags >> 10 & 3;
  pdu->ltx = flags >> 9 & 1;
  pdu->lrx = flags >> 8 & 1;
  pdu->oan = flags >> 6 & 3;
  pdu->otx = flags >> 5 & 1;
  pdu->orx = flags >> 4 & 1;
  pdu->ptx = flags >> 3 & 1;
  pdu->prx = flags >> 2 & 1;
  pdu->dp = flags & 1;

  if (length == 0)
    {
      memset(&pdu->lki, 0, sizeof pdu->lki);
      pdu->llpn = 0;
      memset(&pdu->oki, 0, sizeof pdu->lki);
      pdu->olpn = 0;
      return 1;
    }

  if (length < fixed_length)
    {
      decode_error(b, "bad length in SAK use parameter set");
      return 0;
    }

  if (!decode_key_identifier(b, &pdu->lki) ||
      !decode_uint32(b, &llpnlo) ||
      !decode_key_identifier(b, &pdu->oki) ||
      !decode_uint32(b, &olpnlo) ||
      !decode_any(b, length - fixed_length) ||
      !decode_padding(b))
    return 0;

  pdu->llpn = llpnlo;
  pdu->olpn = olpnlo;
  return 1;
}

static int
encode_distributed_sak(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  uint32_t flags, co, length;
  MacsecCipherSuite cs;

  if (!pdu->has_distributed_plaintext)
    {
      flags = pdu->distributed_an << 10 & 0x0c00;

      switch (pdu->distributed_co)
        {
        case MACSEC_CONFIDENTIALITY_NOT_USED:
          co = 0;
          break;
        case MACSEC_CONFIDENTIALITY_OFFSET_0:
        default:
          co = 1;
          break;
        case MACSEC_CONFIDENTIALITY_OFFSET_30:
          co = 2;
          break;
        case MACSEC_CONFIDENTIALITY_OFFSET_50:
          co = 3;
          break;
        }
      flags |= co << 8 & 0x0300;

      if (!pdu->has_distributed_cs)
        length = 4;
      else
        length = 12;

      if (!pdu->has_distributed_cs)
        cs = MACSEC_CIPHER_SUITE_GCM_AES_128;
      else
        cs = pdu->distributed_cs;

      switch (cs)
        {
        case MACSEC_CIPHER_SUITE_GCM_AES_128:
        case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
          length += 24;
          break;
        default:
          length += 40;
          break;
        }
    }
  else
    {
      length = 0;
      flags = 0;
    }

  if (!encode_header(b, PSET_DISTRIBUTED_SAK, flags, length))
    return 0;

  if (pdu->has_distributed_plaintext)
    return 1;

  return
    encode_uint32(b, pdu->distributed_kn) &&
    (!pdu->has_distributed_cs||encode_cipher_suite(b, pdu->distributed_cs)) &&
    encode_sak_wrap(b, &pdu->distributed_sak) &&
    encode_padding(b);
}

static int
decode_distributed_sak(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length, co, useful_length;
  const int fixed_length = 28;
  MacsecCipherSuite cs;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  pdu->distributed_an = flags >> 10 & 3;
  co = flags >> 8 & 3;
  switch (co)
    {
    case 0:
      pdu->distributed_co = MACSEC_CONFIDENTIALITY_NOT_USED;
      break;
    case 1:
    default:
      pdu->distributed_co = MACSEC_CONFIDENTIALITY_OFFSET_0;
      break;
    case 2:
      pdu->distributed_co = MACSEC_CONFIDENTIALITY_OFFSET_30;
      break;
    case 3:
      pdu->distributed_co = MACSEC_CONFIDENTIALITY_OFFSET_50;
      break;
    }

  if (length == 0)
    {
      if (pdu->distributed_an != 0)
        {
          decode_error(b, "distributed SAK empty but AN nonzero");
          return 0;
        }
      pdu->has_distributed_plaintext = 1;
      return 1;
    }

  if (length < fixed_length)
    {
      decode_error(b, "bad length in distributed SAK parameter set");
      return 0;
    }

  if (!decode_uint32(b, &pdu->distributed_kn))
    {
      decode_error(b, "cannot decode KN of distributed SAK");
      return 0;
    }

  useful_length = fixed_length;

  if (length >= fixed_length + 8)
    {
      if (!decode_cipher_suite(b, &pdu->distributed_cs))
        return 0;
      pdu->has_distributed_cs = 1;
      useful_length += 8;
    }

  if (!pdu->has_distributed_cs)
    cs = MACSEC_CIPHER_SUITE_GCM_AES_128;
  else
    cs = pdu->distributed_cs;

  switch (cs)
    {
    case MACSEC_CIPHER_SUITE_GCM_AES_128:
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
      pdu->distributed_sak.len = 24;
      break;
    default:
      pdu->distributed_sak.len = 40;
      useful_length += 16;
      break;
    }

  return
    decode_sak_wrap(b, &pdu->distributed_sak) &&
    decode_any(b, length - useful_length) &&
    decode_padding(b);
}

static int
encode_distributed_cak(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  uint32_t length;
  const int fixed_length = 24;

  length = fixed_length + pdu->distributed_ckn.len;

  if (!encode_header(b, PSET_DISTRIBUTED_CAK, 0, length))
    return 0;

  return
    encode_cak_wrap(b, &pdu->distributed_cak) &&
    encode_ckn(b, &pdu->distributed_ckn) &&
    encode_padding(b);
}

static int
decode_distributed_cak(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;
  const int fixed_length = 24;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  if (length < fixed_length)
    {
      decode_error(b, "distributed CAK parameter set too small");
      return 0;
    }

  if (length - fixed_length < 1)
    {
      decode_error(b, "distributed CAK name empty");
      return 0;
    }

  if (length - fixed_length > sizeof pdu->ckn.buf)
    {
      decode_error(b, "distributed CAK name too large");
      return 0;
    }

  pdu->distributed_cak.len = 24;
  pdu->distributed_ckn.len = length - fixed_length;

  return
    decode_cak_wrap(b, &pdu->distributed_cak) &&
    decode_ckn(b, &pdu->distributed_ckn) &&
    decode_padding(b);
}

static int
encode_kmd(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  uint32_t length;

  length = pdu->kmd.len;

  if (!encode_header(b, PSET_KMD, 0, length))
    return 0;

  return
    encode_buffer(b, pdu->kmd.buf, pdu->kmd.len) &&
    encode_padding(b);
}

static int
decode_kmd(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  if (length > sizeof pdu->kmd.buf)
    {
      decode_error(b, "key management domain too large");
      return 0;
    }

  pdu->kmd.len = length;

  return
    decode_buffer(b, pdu->kmd.buf, pdu->kmd.len) &&
    decode_padding(b);
}

static int
encode_announcement(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  unsigned char *body;
  unsigned length;

  if (b->pos + 4 > b->len)
    {
      encode_error(b, "no space for header");
      return 0;
    }

  body = b->buf + b->pos + 4;
  length = b->len - b->pos - 4;

  if (!dot1x_encode_announcement(b->intf, pdu->announcement, body, length, &length))
    return 0;

  if (!encode_header(b, PSET_ANNOUNCEMENT, 0, length))
    return 0;

  b->pos += length;

  return encode_padding(b);
}

static int
decode_announcement(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  if (!dot1x_decode_announcement(b->intf, pdu->announcement, b->buf + b->pos, length))
    return 0;

  b->pos += length;

  return decode_padding(b);
}

static int
encode_live_peer_list(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  uint32_t length;
  const Dot1xMkpduPeerList *pl;

  pl = &pdu->lpl;

  length = pl->num * 16;

  if (!encode_header(b, PSET_LIVE_PEER_LIST, 0, length))
    return 0;

  return encode_peer_list(b, pl);
}

static int
decode_live_peer_list(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;
  Dot1xMkpduPeerList *pl;

  pl = &pdu->lpl;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  if ((length & 15) != 0)
    {
      decode_error(b, "invalid live peer list");
      return 0;
    }

  if (length / 16 > sizeof pl->tab / sizeof pl->tab[0])
    {
      decode_error(b, "live peer list too large");
      return 0;
    }

  pl->num = length / 16;

  return decode_peer_list(b, pl);
}

static int
encode_potential_peer_list(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  uint32_t length;
  const Dot1xMkpduPeerList *pl;

  pl = &pdu->ppl;

  length = pl->num * 16;

  if (!encode_header(b, PSET_POTENTIAL_PEER_LIST, 0, length))
    return 0;

  return encode_peer_list(b, pl);
}

static int
decode_potential_peer_list(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;
  Dot1xMkpduPeerList *pl;

  pl = &pdu->ppl;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  if ((length & 15) != 0)
    {
      decode_error(b, "invalid potential peer list");
      return 0;
    }

  if (length / 16 > sizeof pl->tab / sizeof pl->tab[0])
    {
      decode_error(b, "potential peer list too large");
      return 0;
    }

  pl->num = length / 16;

  return decode_peer_list(b, pl);
}

static int
encode_icv_indicator(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  if (!encode_header(b, PSET_ICV_INDICATOR, 0, 16))
    return 0;

  return encode_icv(b, &pdu->icv);
}

static int
decode_icv_indicator(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  if (length != 16)
    {
      decode_error(b, "invalid ICV length");
      return 0;
    }

  return decode_icv(b, &pdu->icv);
}

static int
decode_icv_plain(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  return decode_icv(b, &pdu->icv);
}

static int
decode_ignored(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;

  (void)pdu;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  return
    decode_any(b, length) &&
    decode_padding(b);
}

static int
encode_xpn(WriteBuffer *b, const Dot1xMkpdu *pdu)
{
  uint32_t flags;

  flags = pdu->xpn_mst << 4 & 0x0ff0;

  if (!encode_header(b, PSET_XPN, flags, 8))
    return 0;

  return
      encode_uint32(b, pdu->xpn_llpn) &&
      encode_uint32(b, pdu->xpn_olpn);
}

static int
decode_xpn(ReadBuffer *b, Dot1xMkpdu *pdu)
{
  uint32_t type, flags, length;

  if (!decode_header(b, &type, &flags, &length))
    return 0;

  if (length != 8)
    {
      decode_error(b, "invalid XPN length");
      return 0;
    }

  pdu->xpn_mst = flags >> 4 & 0x00ff;

  return
      decode_uint32(b, &pdu->xpn_llpn) &&
      decode_uint32(b, &pdu->xpn_olpn);
}

static int
encode_header(
  WriteBuffer *b, uint32_t type, uint32_t flags, uint32_t length)
{
  unsigned char oct1, oct2, oct3;

  if (b->pos + 4 + length > b->len)
    {
      encode_error(b, "parameter set too large to encode");
      return 0;
    }

  oct1 = flags >> 4 & 0xff;
  oct2 = flags << 4 & 0xf0;
  oct2 |= length >> 8 & 0x0f;
  oct3 = length & 0xff;

  return
    encode_uint8(b, type) &&
    encode_uint8(b, oct1) &&
    encode_uint8(b, oct2) &&
    encode_uint8(b, oct3);
}

static int
decode_header(
  ReadBuffer *b, uint32_t *type, uint32_t *flags, uint32_t *length)
{
  uint32_t t, f, l;
  uint32_t oct1, oct2, oct3;

  if (!(decode_uint8(b, &t) &&
        decode_uint8(b, &oct1) &&
        decode_uint8(b, &oct2) &&
        decode_uint8(b, &oct3)))
    return 0;

  f = oct1 << 4;
  f |= oct2 >> 4;
  l = oct2 << 8 & 0xf00;
  l |= oct3;

  if (b->pos + l > b->len)
    {
      decode_error(b, "truncated parameter set");
      return 0;
    }

  *type = t;
  *flags = f;
  *length = l;
  return 1;
}

static int
encode_peer_list(WriteBuffer *b, const Dot1xMkpduPeerList *pl)
{
  const Dot1xMkpduPeer *p;
  int i;

  for (i = 0; i < pl->num; i++)
    {
      p = &pl->tab[i];
      if (!encode_member_identifier(b, &p->mi) ||
          !encode_uint32(b, p->mn))
        return 0;
    }
  return 1;
}

static int
decode_peer_list(ReadBuffer *b, Dot1xMkpduPeerList *pl)
{
  Dot1xMkpduPeer *p;
  int i;

  for (i = 0; i < pl->num; i++)
    {
      p = &pl->tab[i];
      if (!decode_member_identifier(b, &p->mi) ||
          !decode_uint32(b, &p->mn))
        return 0;
    }
  return 1;
}

static int
encode_mka_algorithm(WriteBuffer *b, Dot1xMkaAlgorithm ma)
{
  switch (ma)
    {
    case DOT1X_MKA_ALGORITHM_8021X_2009:
      return encode_buffer(b,
                           mka_algorithm_8021x_2009,
                           sizeof mka_algorithm_8021x_2009);

    default:
      encode_error(b, "cannot encode MKA algorithm");
      return 0;
    }

  return 1;
}

static int
decode_mka_algorithm(ReadBuffer *b, Dot1xMkaAlgorithm *ma)
{
  static unsigned char buf[4];
  Dot1xFormatBuffer fb;

  if (!decode_buffer(b, buf, sizeof buf))
    return 0;

  if (sizeof buf != sizeof mka_algorithm_8021x_2009 ||
      memcmp(buf, mka_algorithm_8021x_2009, sizeof buf))
    {
      decode_error(
        b, "cannot decode MKA algorithm %s",
        dot1x_format_buf(&fb, buf, sizeof buf));
      return 0;
    }

  *ma = DOT1X_MKA_ALGORITHM_8021X_2009;
  return 1;
}

static int
encode_cipher_suite(WriteBuffer *b, MacsecCipherSuite cs)
{
  const unsigned char (*refnr)[8];

  if (!dot1x_get_cipher_suite_refnr(&refnr, cs))
    {
      encode_error(b, "cannot encode MACsec cipher suite");
      return 0;
    }

  return encode_buffer(b, *refnr, 8);
}

static int
decode_cipher_suite(ReadBuffer *b, MacsecCipherSuite *cs)
{
  static unsigned char buf[8];
  Dot1xFormatBuffer fb;

  if (!decode_buffer(b, buf, sizeof buf))
    return 0;

  if (!dot1x_get_cipher_suite_ident(cs, &buf))
    {
      decode_error(
        b, "cannot decode MACsec cipher suite %s",
        dot1x_format_buf(&fb, buf, sizeof buf));
      return 0;
    }

  return 1;
}

static int
encode_sci(WriteBuffer *b, const MacsecSci *sci)
{
  return
    encode_buffer(b, sci->address.buf, sizeof sci->address.buf) &&
    encode_uint16(b, sci->portid);
}

static int
decode_sci(ReadBuffer *b, MacsecSci *sci)
{
  return
    decode_buffer(b, sci->address.buf, sizeof sci->address.buf) &&
    decode_uint16(b, &sci->portid);
}

static int
encode_key_identifier(WriteBuffer *b, const Dot1xKeyIdentifier *ki)
{
  return
    encode_member_identifier(b, &ki->mi) &&
    encode_uint32(b, ki->kn);
}

static int
decode_key_identifier(ReadBuffer *b, Dot1xKeyIdentifier *ki)
{
  return
    decode_member_identifier(b, &ki->mi) &&
    decode_uint32(b, &ki->kn);
}

static int
encode_member_identifier(WriteBuffer *b, const Dot1xMemberIdentifier *mi)
{
  return encode_buffer(b, (void *)mi, sizeof *mi);
}

static int
decode_member_identifier(ReadBuffer *b, Dot1xMemberIdentifier *mi)
{
  return decode_buffer(b, (void *)mi, sizeof *mi);
}

static int
encode_ckn(WriteBuffer *b, const Dot1xCkn *ckn)
{
  return encode_buffer(b, ckn->buf, ckn->len);
}

static int
decode_ckn(ReadBuffer *b, Dot1xCkn *ckn)
{
  return decode_buffer(b, ckn->buf, ckn->len);
}

static int
encode_cak_wrap(WriteBuffer *b, const Dot1xCryptoWrap *ckw)
{
  if (ckw->len != 24)
    {
      encode_error(b, "cannot encode CAK wrap");
      return 0;
    }

  return encode_buffer(b, ckw->buf, ckw->len);
}

static int
decode_cak_wrap(ReadBuffer *b, Dot1xCryptoWrap *ckw)
{
  return decode_buffer(b, ckw->buf, ckw->len);
}

static int
encode_sak_wrap(WriteBuffer *b, const Dot1xCryptoWrap *skw)
{
  if (skw->len != 24 && skw->len != 40)
    {
      encode_error(b, "cannot encode SAK wrap");
      return 0;
    }

  return encode_buffer(b, skw->buf, skw->len);
}

static int
decode_sak_wrap(ReadBuffer *b, Dot1xCryptoWrap *skw)
{
  return decode_buffer(b, skw->buf, skw->len);
}

static int
encode_icv(WriteBuffer *b, const Dot1xIcv *icv)
{
  return encode_buffer(b, (void *)icv, sizeof *icv);
}

static int
decode_icv(ReadBuffer *b, Dot1xIcv *icv)
{
  return decode_buffer(b, (void *)icv, sizeof *icv);
}

static int
encode_uint32(WriteBuffer *b, uint32_t n)
{
  uint32_t oct3, oct2, oct1, oct0;

  oct3 = n >> 24 & 0xff;
  oct2 = n >> 16 & 0xff;
  oct1 = n >> 8 & 0xff;
  oct0 = n & 0xff;

  return
    encode_uint8(b, oct3) &&
    encode_uint8(b, oct2) &&
    encode_uint8(b, oct1) &&
    encode_uint8(b, oct0);
}

static int
decode_uint32(ReadBuffer *b, uint32_t *n)
{
  uint32_t oct3, oct2, oct1, oct0;

  if (!(decode_uint8(b, &oct3) &&
        decode_uint8(b, &oct2) &&
        decode_uint8(b, &oct1) &&
        decode_uint8(b, &oct0)))
    return 0;

  *n = oct3 << 24;
  *n |= oct2 << 16;
  *n |= oct1 << 8;
  *n |= oct0;
  return 1;
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
encode_padding(WriteBuffer *b)
{
  return encode_null(b, (4 - (b->pos & 3)) & 3);
}

static int
decode_padding(ReadBuffer *b)
{
  return decode_any(b, (4 - (b->pos & 3)) & 3);
}

static int
encode_buffer(WriteBuffer *b, const unsigned char *buf, unsigned len)
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
encode_null(WriteBuffer *b, unsigned len)
{
  if (b->pos + len > b->len)
    {
      encode_error(b, "out of output space");
      return 0;
    }

  memset(b->buf + b->pos, 0, len);
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
  dot1x_error(b->intf, "MKA encode: %s", dot1x_format_vprintf(&fb, fmt, ap));
  va_end(ap);
}

static void
decode_error(ReadBuffer *b, const char *fmt, ...)
{
  va_list ap;
  Dot1xFormatBuffer fb;

  va_start(ap, fmt);
  dot1x_error(b->intf, "MKA decode: %s", dot1x_format_vprintf(&fb, fmt, ap));
  va_end(ap);
}
