/**

   @copyright
   Copyright (c) 2012 - 2021, Rambus Inc. All rights reserved.

   File: softsec.c

   @description
   Software-based MACsec (802.1AE) protocol.

*/


#include "softsec.h"
#include "macsec_clib.h"
#include "stdlib.h"
#include "ctype.h"

typedef struct {
  unsigned char *buf;
  unsigned len;
} WriteBuffer;

typedef struct {
  const unsigned char *buf;
  unsigned len;
} ReadBuffer;

#ifdef SOFTSEC_SOFTWARE_CRYPTO
typedef struct {
  const unsigned char *buf;
  unsigned len;
} TestVector;
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

/*
 * Clear the contents and associated crypto data of a peer object.
 */
static void
clear_peer(SoftsecPeer *peer);

/*
 * Update the MAC_Enabled, MAC_Operational and operPointToPointMAC
 * status variables of a controlled port.
 */
static void
update_controlled_mac(SoftsecIntf *intf, unsigned portid);

/*
 * Get pointer to the port object of the real port (portid less than
 * 2) or a virtual port (portid greater than one). As a special case,
 * a large portid (greater than 1 + SOFTSEC_MAX_PEERS) maps to the
 * real port to enable outbound self tests that use a large portid.
 */
static SoftsecPort *
get_port(const SoftsecIntf *intf, unsigned portid);

/*
 * Initialize SecY control variables.
 */
static void
init_secy_ctrl(MacsecSecyCtrl *ctrl);

/*
 * Frame encoding and decoding functions.
 */

static inline int
put_uint8(WriteBuffer *b, unsigned value);

static inline int
get_uint8(ReadBuffer *b, unsigned *value);

static inline int
put_uint16(WriteBuffer *b, unsigned value);

static inline int
get_uint16(ReadBuffer *b, unsigned *value);

static inline int
put_uint32(WriteBuffer *b, uint32_t value);

static inline int
get_uint32(ReadBuffer *b, uint32_t *value);

static inline int
put_buffer(WriteBuffer *b, const unsigned char *buf, unsigned len);

static inline int
get_buffer(ReadBuffer *b, unsigned char *buf, unsigned len);

static inline int
put_space(WriteBuffer *b, unsigned char **ptr, unsigned len);

static inline int
put_skip(WriteBuffer *b, unsigned len);

static inline int
get_skip(ReadBuffer *b, unsigned len);

static inline int
get_pointer(ReadBuffer *b, const unsigned char **ptr, unsigned len);

static inline int
get_pointer_tail(
  ReadBuffer *b, const unsigned char **ptr, unsigned len);

#ifdef SOFTSEC_SOFTWARE_CRYPTO
/*
 * Test functions.
 */
static unsigned
test_integrity_54(void);

static unsigned
test_integrity_60(void);

static unsigned
test_integrity_65(void);

static unsigned
test_integrity_79(void);

static unsigned
test_confidentiality_54(void);

static unsigned
test_confidentiality_60(void);

static unsigned
test_confidentiality_61(void);

static unsigned
test_confidentiality_75(void);

static unsigned
test_bidirectional(
  TestVector key,
  TestVector ssci,
  TestVector pnhi,
  TestVector salt,
  TestVector unprotected,
  TestVector header,
  TestVector secure,
  TestVector icv);
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

/*
 * Broadcast address.
 */
static const MacsecAddress
broadcast_address = {.buf = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

/*
 * Public functions.
 */

void
softsec_get_capabilities(
  MacsecCapabilities *capabilities)
{
  SoftsecPort *port = (void *)0;
  int i;

  for (i = 0; i < MACSEC_CIPHER_SUITE_COUNT; i++)
    {
      capabilities->cipher_suites[i].implemented = 1;
      capabilities->cipher_suites[i].capability =
        MACSEC_CAPABILITY_CONFIDENTIALITY_WITH_OFFSET;
    }

  capabilities->max_peer_scs = SOFTSEC_MAX_PEERS;
  capabilities->max_receive_keys = sizeof port->dkeys / sizeof port->dkeys[0];
  capabilities->max_transmit_keys = sizeof port->dkeys / sizeof port->dkeys[0];
}

void
softsec_init_intf(
  SoftsecIntf *intf,
  const MacsecAddress *address)
{
  memset(intf, 0, sizeof *intf);

  intf->address = *address;

  softsec_peers_init(&intf->peers);

  init_secy_ctrl(&intf->rport.ctrl);

  intf->rport.transmit_sc.stat.sci.address = *address;
  intf->rport.transmit_sc.stat.sci.portid = 1;

  intf->rport.transmit_sc.stat.created_time = intf->sysuptime;
  intf->rport.transmit_sc.stat.started_time = intf->sysuptime;
  intf->rport.transmit_sc.stat.stopped_time = intf->sysuptime;
}

void
softsec_uninit_intf(
  SoftsecIntf *intf)
{
  softsec_peers_foreach(&intf->peers, clear_peer);

  memset(intf, 0, sizeof *intf);
}

void
softsec_set_common_mac_state(
  SoftsecIntf *intf,
  unsigned enabled,
  unsigned operational,
  unsigned point_to_point)
{
  int i;

  intf->mac_enabled = enabled;
  intf->mac_operational = operational;
  intf->oper_point_to_point_mac = point_to_point;

  update_controlled_mac(intf, 1);
  for (i = 2; i < sizeof intf->vport_peer / sizeof intf->vport_peer[0]; i++)
    if (intf->vport_peer[i])
      update_controlled_mac(intf, i);
}

void
softsec_set_controlled_port_enabled(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned value)
{
  SoftsecPort *port = get_port(intf, portid);

  port->controlled_port_enabled = value;
  update_controlled_mac(intf, portid);
}

void
softsec_set_sysuptime(
  SoftsecIntf *intf,
  uint32_t sysuptime)
{
  intf->sysuptime = sysuptime;
}

SoftsecPeer *
softsec_create_peer(
  SoftsecIntf *intf,
  unsigned portid,
  const MacsecAddress *peer_address,
  unsigned peer_portid)
{
  SoftsecPeer *peer = NULL, *peer2;

  if (!(peer = softsec_peers_insert(&intf->peers, peer_address)))
    goto fail;

  assert(portid < 2 + SOFTSEC_MAX_PEERS);

  if (portid > 1)
    {
      if (intf->vport_peer[portid])
        goto fail;
      intf->vport_peer[portid] = peer;

      init_secy_ctrl(&peer->vport.ctrl);

      peer->vport.transmit_sc.stat.sci.address = intf->address;
      peer->vport.transmit_sc.stat.sci.portid = portid;

      peer->vport.transmit_sc.stat.created_time = intf->sysuptime;
      peer->vport.transmit_sc.stat.started_time = intf->sysuptime;
      peer->vport.transmit_sc.stat.stopped_time = intf->sysuptime;
    }

  peer->intf = intf;
  peer->portid = portid;
  peer->peer_portid = peer_portid;

  peer->receive_sc.stat.sci.address = *peer_address;
  peer->receive_sc.stat.sci.portid = peer_portid;

  peer->receive_sc.stat.created_time = intf->sysuptime;
  peer->receive_sc.stat.started_time = intf->sysuptime;
  peer->receive_sc.stat.stopped_time = intf->sysuptime;

  return peer;

 fail:
  if (peer)
    {
      peer2 = softsec_peers_remove(&intf->peers, peer_address);
      assert(peer2 == peer);
    }
  return NULL;
}

void
softsec_destroy_peer(
  SoftsecPeer *peer)
{
  SoftsecIntf *intf = peer->intf;
  SoftsecPeer *peer2;

  if (peer->portid > 1)
    {
      assert(intf->vport_peer[peer->portid] == peer);
      intf->vport_peer[peer->portid] = NULL;
    }

  peer2 = softsec_peers_remove(&intf->peers, &peer->peer_address);
  assert(peer2 == peer);

  /* Update the status variables of the port only in the case of real port (portid < 2) */
  if (peer->portid <= 1)
    update_controlled_mac(intf, peer->portid);

  clear_peer(peer);
}

void
softsec_get_receive_sc_stat(
  SoftsecPeer *peer,
  MacsecReceiveScStat *stat)
{
  *stat = peer->receive_sc.stat;
}

void
softsec_set_secy_ctrl(
  SoftsecIntf *intf,
  unsigned portid,
  const MacsecSecyCtrl *ctrl)
{
  SoftsecPort *port;

  port = get_port(intf, portid);
  port->ctrl = *ctrl;
  update_controlled_mac(intf, portid);
}

void
softsec_get_secy_ctrl(
  const SoftsecIntf *intf,
  unsigned portid,
  MacsecSecyCtrl *ctrl)
{
  SoftsecPort *port;

  port = get_port(intf, portid);
  *ctrl = port->ctrl;
}

void
softsec_get_secy_stat(
  const SoftsecIntf *intf,
  unsigned portid,
  MacsecSecyStat *stat)
{
  SoftsecPort *port;

  port = get_port(intf, portid);
  *stat = port->stat;
}

void
softsec_get_port_stat(
  const SoftsecIntf *intf,
  unsigned portid,
  MacsecPortStat *stat)
{
  SoftsecPort *port;

  port = get_port(intf, portid);
  *stat = port->pstat;
}

void
softsec_get_transmit_sc_stat(
  const SoftsecIntf *intf,
  unsigned portid,
  MacsecTransmitScStat *stat)
{
  SoftsecPort *port;

  port = get_port(intf, portid);
  *stat = port->transmit_sc.stat;
}

void
softsec_get_latest_an(
        SoftsecIntf *intf,
        unsigned portid,
        unsigned *latest_an)
{
  SoftsecPort *port;

  port = get_port(intf, portid);

  if (port != NULL)
    {
      *latest_an = port->transmit_sc.an;
    }
}

#ifdef SOFTSEC_SOFTWARE_CRYPTO
SoftsecDkey *
softsec_install_key(
  SoftsecIntf *intf,
  unsigned portid,
  const MacsecKey *sak,
  const MacsecKi *ki,
  const MacsecSalt *salt)
{
  SoftsecPort *port;
  SoftsecDkey *dkey;

  assert(sak->len == 16 || sak->len == 32);

  port = get_port(intf, portid);

  dkey = &port->dkeys[port->next_dkey];
  port->next_dkey ^= 1;

  dkey->ki = *ki;
  softsec_crypto_init(&dkey->crypto, sak);
  if (salt)
    {
      assert(sizeof dkey->salthi + sizeof dkey->saltlo == sizeof salt->buf);
      memcpy(&dkey->salthi, salt->buf, sizeof dkey->salthi);
      memcpy(
        &dkey->saltlo, salt->buf + sizeof dkey->salthi, sizeof dkey->saltlo);
    }
  return dkey;
}

SoftsecDkey *
softsec_get_dkey(
        SoftsecIntf *intf,
        unsigned portid,
        const MacsecKi *ki)
{
  SoftsecPort *port;
  SoftsecDkey *dkey = NULL;
  int i;

  port = get_port(intf, portid);

  for (i = 0; i < sizeof port->dkeys / sizeof port->dkeys[0]; i++)
    {
      dkey = &port->dkeys[i];
      if (memcmp(&dkey->ki, ki, sizeof *ki) == 0)
          break;
    }

  return dkey;
}
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

SoftsecDkey *
softsec_install_key_handle(
  SoftsecIntf *intf,
  unsigned portid,
  void *crypto_handle,
  const MacsecKi *ki,
  const MacsecSalt *salt)
{
  SoftsecPort *port;
  SoftsecDkey *dkey;

  port = get_port(intf, portid);

  dkey = &port->dkeys[port->next_dkey];
  port->next_dkey ^= 1;

  dkey->ki = *ki;
  dkey->crypto_handle = crypto_handle;
  if (salt)
    {
      assert(sizeof dkey->salthi + sizeof dkey->saltlo == sizeof salt->buf);
      memcpy(&dkey->salthi, salt->buf, sizeof dkey->salthi);
      memcpy(
        &dkey->saltlo, salt->buf + sizeof dkey->salthi, sizeof dkey->saltlo);
    }
  return dkey;
}

void
softsec_create_transmit_sa(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned an,
  uint64_t next_pn,
  SoftsecDkey *dkey,
  const MacsecSsci *ssci)
{
  SoftsecPort *port;
  SoftsecTransmitSc *tsc;
  SoftsecTransmitSa *tsa;

  port = get_port(intf, portid);
  tsc = &port->transmit_sc;

  assert(an <= 3);

  tsa = &tsc->sas[an];

  tsa->enable = 0;
  tsa->next_pn = next_pn;
  tsa->dkey = dkey;
  if (ssci)
    {
      assert(sizeof tsa->ssci == sizeof ssci->buf);
      memcpy(&tsa->ssci, ssci->buf, sizeof ssci->buf);
    }

  memset(&tsa->stat, 0, sizeof tsa->stat);
  tsa->stat.created_time = intf->sysuptime;
  tsa->stat.started_time = intf->sysuptime;
  tsa->stat.stopped_time = intf->sysuptime;
}

void
softsec_enable_transmit(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned an)
{
  SoftsecPort *port;
  SoftsecTransmitSc *tsc;
  SoftsecTransmitSa *tsa, *oldtsa = NULL;

  port = get_port(intf, portid);
  tsc = &port->transmit_sc;

  tsa = &tsc->sas[an];

  if (tsc->an != an)
    oldtsa = &tsc->sas[tsc->an];

  tsa->enable = 1;
  tsa->stat.in_use = 1;
  tsa->stat.started_time = intf->sysuptime;

  tsc->an = an;

  if (oldtsa && oldtsa->enable)
    {
      oldtsa->enable = 0;
      oldtsa->stat.in_use = 0;
      oldtsa->stat.stopped_time = intf->sysuptime;
    }

  if (!tsc->stat.transmitting)
    {
      tsc->stat.transmitting = 1;
      tsc->stat.started_time = intf->sysuptime;
      update_controlled_mac(intf, portid);
    }
  tsc->stat.encoding_sa = tsc->an;
  tsc->stat.enciphering_sa = tsc->an;
}

void
softsec_get_next_pn(
  const SoftsecIntf *intf,
  unsigned portid,
  unsigned an,
  uint64_t *next_pn)
{
  SoftsecPort *port;
  SoftsecTransmitSa *tsa;

  port = get_port(intf, portid);
  tsa = &port->transmit_sc.sas[an];

  *next_pn = tsa->next_pn;
}

void
softsec_get_transmit_sa_stat(
  const SoftsecIntf *intf,
  unsigned portid,
  unsigned an,
  MacsecTransmitSaStat *stat)
{
  SoftsecPort *port;

  port = get_port(intf, portid);
  *stat = port->transmit_sc.sas[an].stat;
}

void
softsec_create_receive_sa(
  SoftsecPeer *peer,
  unsigned an,
  uint64_t lowest_pn,
  SoftsecDkey *dkey,
  const MacsecSsci *ssci)
{
  SoftsecIntf *intf = peer->intf;
  SoftsecReceiveSc *rsc = &peer->receive_sc;
  SoftsecReceiveSa *rsa;

  assert(an <= 3);

  rsa = &rsc->sas[an];

  rsa->enable = 0;
  rsa->lowest_pn = lowest_pn;
  rsa->next_pn = lowest_pn;
  rsa->dkey = dkey;
  if (ssci)
    {
      assert(sizeof rsa->ssci == sizeof ssci->buf);
      memcpy(&rsa->ssci, ssci->buf, sizeof ssci->buf);
    }

  memset(&rsa->stat, 0, sizeof rsa->stat);
  rsa->stat.created_time = intf->sysuptime;
  rsa->stat.started_time = intf->sysuptime;
  rsa->stat.stopped_time = intf->sysuptime;
}

void
softsec_enable_receive(
  SoftsecPeer *peer,
  unsigned an)
{
  SoftsecIntf *intf = peer->intf;
  SoftsecReceiveSc *rsc = &peer->receive_sc;
  SoftsecReceiveSa *rsa, *oldrsa = NULL, *oldoldrsa = NULL;

  rsa = &rsc->sas[an];

  if (rsc->an != an)
    oldrsa = &rsc->sas[rsc->an];

  if (rsc->oan != an && rsc->oan != rsc->an)
    oldoldrsa = &rsc->sas[rsc->oan];

  rsa->enable = 1;
  rsa->stat.in_use = 1;
  rsa->stat.started_time = intf->sysuptime;

  rsc->oan = rsc->an;
  rsc->an = an;

  /* ensure that no more than two SAs remain enabled */
  if (oldrsa && oldoldrsa && oldoldrsa->enable)
    {
      oldoldrsa->enable = 0;
      oldoldrsa->stat.in_use = 0;
      oldoldrsa->stat.stopped_time = intf->sysuptime;
    }

  if (!rsc->stat.receiving)
    {
      rsc->stat.receiving = 1;
      rsc->stat.started_time = intf->sysuptime;
      update_controlled_mac(intf, peer->portid);
    }
}

void
softsec_set_lowest_pn(
  SoftsecPeer *peer,
  unsigned an,
  uint64_t lowest_pn)
{
  SoftsecReceiveSc *rsc = &peer->receive_sc;
  SoftsecReceiveSa *rsa = &rsc->sas[an];

  rsa->lowest_pn = lowest_pn;
  if (rsa->next_pn < lowest_pn)
    rsa->next_pn = lowest_pn;
}

void
softsec_get_receive_sa_stat(
  const SoftsecPeer *peer,
  unsigned an,
  MacsecReceiveSaStat *stat)
{
  *stat = peer->receive_sc.sas[an].stat;
}

#ifdef SOFTSEC_SOFTWARE_CRYPTO
unsigned
softsec_output(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned char *dstbuf, unsigned dstmax,
  const unsigned char *srcbuf, unsigned srclen)
{
  SoftsecOutputDescriptor od;
  SoftsecCryptoContext *crypto;
  void *crypto_handle;
  SoftsecCryptoWriteBytes ciphertext;
  SoftsecCryptoTag *tag;
  const SoftsecCryptoIv *iv;
  SoftsecCryptoReadBytes plaintext;
  SoftsecCryptoReadBytes aad;
  SoftsecOutputState state;

  state = softsec_output_begin(
    &od, intf, portid, dstbuf, dstmax, srcbuf, srclen);

  if (state == SOFTSEC_OUTPUT_ENCRYPT)
    {
      crypto = &od.tsa->dkey->crypto;

      softsec_get_encryption_parameters(
        &od, &crypto_handle, &ciphertext, &tag, &iv, &plaintext, &aad);

      softsec_crypto_encrypt(
        crypto, &ciphertext, tag, iv, &plaintext, &aad);
    }

  return softsec_output_end(&od);
}

unsigned
softsec_input(
  SoftsecIntf *intf,
  unsigned *portid,
  unsigned char *dstbuf, unsigned dstmax,
  const unsigned char *srcbuf, unsigned srclen)
{
  SoftsecInputDescriptor id;
  SoftsecCryptoContext *crypto;
  void *crypto_handle;
  unsigned *authentic;
  SoftsecCryptoWriteBytes plaintext;
  const SoftsecCryptoIv *iv;
  SoftsecCryptoReadBytes ciphertext;
  SoftsecCryptoReadBytes aad;
  const SoftsecCryptoTag *tag;
  SoftsecInputState state;

  state = softsec_input_begin(&id, intf, dstbuf, dstmax, srcbuf, srclen);

  if (state == SOFTSEC_INPUT_DECRYPT)
    {
      crypto = &id.rsa->dkey->crypto;

      softsec_get_decryption_parameters(
        &id, &crypto_handle,
        &authentic, &plaintext, &iv, &ciphertext, &aad, &tag);

      softsec_crypto_decrypt(
        crypto, authentic, &plaintext, iv, &ciphertext, &aad, tag);
    }

  return softsec_input_end(&id, portid);
}
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

SoftsecOutputState
softsec_output_begin(
  SoftsecOutputDescriptor *od,
  SoftsecIntf *intf,
  unsigned portid,
  unsigned char *dstbuf, unsigned dstmax,
  const unsigned char *srcbuf, unsigned srclen)
{
  WriteBuffer b = {dstbuf, dstmax};
  SoftsecPort *port;
  SoftsecTransmitSc *tsc;
  SoftsecTransmitSa *tsa;
  SoftsecPeer *peer = NULL;
  const unsigned char *dmac, *smac;
  unsigned char *secdata, *u8p;
  unsigned srchdrlen, dsthdrlen, datalen, confoff;
  unsigned tci_an, sl;
  unsigned vbit, esbit, scbit, scbbit, ebit, cbit, an;
  uint64_t u64;
  uint32_t u32;

  od->intf = intf;
  od->portid = portid;
  od->dstbuf = dstbuf;
  od->dstmax = dstmax;
  od->srcbuf = srcbuf;

  /* get real or virtual port and check use of extended packet numbers */
  port = get_port(intf, portid);
  switch (port->ctrl.current_cipher_suite)
    {
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_256:
      od->xpn = 1;
      break;
    default:
      od->xpn = 0;
      break;
    }

  /* pass frame unchanged if protection disabled */
  if (!port->ctrl.protect_frames)
    {
      if (dstmax < srclen)
        goto toolong;

      memcpy(dstbuf, srcbuf, srclen);
      od->dstlen = srclen;
      port->stat.out_pkts_untagged++;
      od->state = SOFTSEC_OUTPUT_PASS;
      return od->state;
    }

  /* drop everything without a valid ethernet header */
  if (srclen < 14)
    goto drop;

  /* point to addresses and data */
  dmac = srcbuf;
  smac = srcbuf + 6;

  /* header and data lengths */
  srchdrlen = 12;
  datalen = srclen - 12;

  /* get transmit SC and SA */
  tsc = &port->transmit_sc;
  tsa = &tsc->sas[tsc->an];
  if (!tsa->enable)
    goto drop;
  od->crypto_handle = tsa->dkey->crypto_handle;

  /* add addresses and ethertype */
  if (!put_buffer(&b, dmac, 6) ||
      !put_buffer(&b, smac, 6) ||
      !put_uint16(&b, 0x88e5))
    goto toolong;

  /* MACsec version */
  vbit = 0;

  /* choose explicit, end system or point-to-point SCI encoding: */
  /* explicit if local port is virtual or */
  if (portid > 1 ||
      /* configured to always use explicit SCI or */
      port->ctrl.always_include_sci ||
      /* not end system or SCB encoding and */
      (!port->ctrl.use_es && !port->ctrl.use_scb &&
       /* there are multiple peers or */
       (softsec_peers_count(&intf->peers) > 1 ||
        /* there is only one peer but it is using virtual ports */
        ((peer = softsec_peers_next(&intf->peers, NULL)) &&
         peer->peer_portid > 1))))
    {
      esbit = 0;
      scbit = 1;
    }
  /* end system encoding if so configured */
  else if (port->ctrl.use_es)
    {
      /* cannot use end system encoding for bridged frames */
      if (memcmp(smac, intf->address.buf, 6))
        goto drop;
      esbit = 1;
      scbit = 0;
    }
  /* otherwise point-to-point, i.e. single peer and no virtual ports */
  else
    {
      esbit = 0;
      scbit = 0;
    }

  /* set SCB bit if required */
  if (port->ctrl.use_scb)
    scbbit = 1;
  else
    scbbit = 0;

  /* confidentiality or integrity only */
  if (port->ctrl.confidentiality_offset != MACSEC_CONFIDENTIALITY_NOT_USED)
    {
      ebit = 1;
      cbit = 1;
    }
  else
    {
      ebit = 0;
      cbit = 0;
    }

  /* SA number */
  an = tsc->an;

  /* construct TCI/AN */
  tci_an = vbit << 7;
  tci_an |= esbit << 6;
  tci_an |= scbit << 5;
  tci_an |= scbbit << 4;
  tci_an |= ebit << 3;
  tci_an |= cbit << 2;
  tci_an |= an;

  /* short length */
  if (datalen < 48)
    sl = datalen;
  else
    sl = 0;

  /* add TCI/AN, SL, PN */
  if (!put_uint8(&b, tci_an) ||
      !put_uint8(&b, sl) ||
      !put_uint32(&b, (uint32_t)tsa->next_pn))
    goto toolong;

  /* add explicit SCI */
  if (scbit)
    {
      if (!put_buffer(&b, intf->address.buf, 6))
        goto toolong;
      if (!put_uint16(&b, portid))
        goto toolong;
    }

  /* ensure space for secure data */
  if (!put_space(&b, &secdata, datalen))
    goto toolong;

  /* mark end of header */
  dsthdrlen = secdata - dstbuf;

  if (!od->xpn)
    {
      /* create 96-bit IV from SCI and PN */
      memcpy(od->iv, intf->address.buf, 6);
      if (!scbbit)
        {
          od->iv[6] = (unsigned char)(portid >> 8);
          od->iv[7] = (unsigned char)portid;
        }
      else
        {
          od->iv[6] = 0;
          od->iv[7] = 0;
        }
      memcpy(od->iv + 8, dstbuf + 16, 4);
    }
  else
    {
      /* create 96-bit IV from SSCI, salt and 64-bit PN */
      u32 = tsa->ssci ^ tsa->dkey->salthi;
      u8p = (void *)&u64;
      *u8p++ = ((tsa->next_pn & 0xff00000000000000ULL) >> 56);
      *u8p++ = ((tsa->next_pn & 0x00ff000000000000ULL) >> 48);
      *u8p++ = ((tsa->next_pn & 0x0000ff0000000000ULL) >> 40);
      *u8p++ = ((tsa->next_pn & 0x000000ff00000000ULL) >> 32);
      *u8p++ = ((tsa->next_pn & 0x00000000ff000000ULL) >> 24);
      *u8p++ = ((tsa->next_pn & 0x0000000000ff0000ULL) >> 16);
      *u8p++ = ((tsa->next_pn & 0x000000000000ff00ULL) >> 8);
      *u8p++ = ((tsa->next_pn & 0x00000000000000ffULL) >> 0);
      u64 ^= tsa->dkey->saltlo;
      memcpy(od->iv, &u32, 4);
      memcpy(od->iv + 4, &u64, 8);
    }

  /* determine length of and copy integrity-only user data */
  switch (port->ctrl.confidentiality_offset)
    {
    default:
    case MACSEC_CONFIDENTIALITY_OFFSET_0:
      confoff = 0;
      break;
    case MACSEC_CONFIDENTIALITY_OFFSET_30:
      confoff = 30;
      break;
    case MACSEC_CONFIDENTIALITY_OFFSET_50:
      confoff = 50;
      break;
    case MACSEC_CONFIDENTIALITY_NOT_USED:
      confoff = datalen;
      break;
    }
  if (!ebit || confoff > datalen)
    confoff = datalen;
  memcpy(dstbuf + dsthdrlen, srcbuf + srchdrlen, confoff);

  od->dstauth = dsthdrlen + confoff;
  od->srcskip = srchdrlen + confoff;
  od->encrlen = datalen - confoff;

  od->tsc = tsc;
  od->tsa = tsa;
  od->ebit = ebit;
  od->state = SOFTSEC_OUTPUT_ENCRYPT;
  return od->state;

 toolong:
  port->stat.out_pkts_too_long++;
  port->pstat.if_out_errors++;
 drop:
  od->state = SOFTSEC_OUTPUT_DROP;
  return od->state;
}

unsigned
softsec_output_end(
  SoftsecOutputDescriptor *od)
{
  WriteBuffer b = {od->dstbuf, od->dstmax};
  SoftsecPort *port = get_port(od->intf, od->portid);
  SoftsecTransmitSc *tsc = od->tsc;
  SoftsecTransmitSa *tsa = od->tsa;
  unsigned dstlen;

  if (od->state == SOFTSEC_OUTPUT_PASS)
    {
      dstlen = od->dstlen;
      goto pass;
    }
  else if (od->state != SOFTSEC_OUTPUT_ENCRYPT)
    return 0;

  /* point to end of secure data */
  if (!put_skip(&b, od->dstauth + od->encrlen))
    goto toolong;

  /* add ICV */
  if (!put_buffer(&b, od->icv, sizeof od->icv))
    goto toolong;

  /* update protection octet statistics */
  port->stat.out_octets_protected += od->dstauth;
  port->stat.out_octets_encrypted += od->encrlen;

  /* update transmit packet number */
  if ((!od->xpn && (uint32_t)tsa->next_pn >= 0xffffffff) ||
      (od->xpn && tsa->next_pn >= 0xffffffffffffffffULL))
    {
      tsa->enable = 0;
      tsa->stat.in_use = 0;
      tsa->stat.stopped_time = od->intf->sysuptime;
      if (tsc->stat.transmitting)
        {
          tsc->stat.transmitting = 0;
          tsc->stat.stopped_time = od->intf->sysuptime;
          update_controlled_mac(od->intf, od->portid);
        }
    }
  else
    {
      char *pn_increment = getenv("PN_INCREMENT");

      if (pn_increment != NULL && isdigit(*pn_increment))
        tsa->next_pn += atoi(pn_increment);
      else
        tsa->next_pn++;
      tsa->stat.next_pn = tsa->next_pn;
    }

  /* count it */
  if (od->ebit)
    {
      tsa->stat.out_pkts_encrypted++;
      tsc->stat.out_pkts_encrypted++;
    }
  else
    {
      tsa->stat.out_pkts_protected++;
      tsc->stat.out_pkts_protected++;
    }

  dstlen = od->dstmax - b.len;

 pass:
  port->pstat.if_out_octets += dstlen;
  if (!(od->dstbuf[0] & 1))
    port->pstat.if_out_ucast_pkts++;
  else if (!memcmp(od->dstbuf, &broadcast_address, 6))
    port->pstat.if_out_broadcast_pkts++;
  else
    port->pstat.if_out_multicast_pkts++;
  return dstlen;

 toolong:
  port->stat.out_pkts_too_long++;
  return 0;
}

SoftsecInputState
softsec_input_begin(
  SoftsecInputDescriptor *id,
  SoftsecIntf *intf,
  unsigned char *dstbuf, unsigned dstmax,
  const unsigned char *srcbuf, unsigned srclen)
{
  ReadBuffer b = {srcbuf, srclen};
  SoftsecPort *port;
  SoftsecReceiveSc *rsc;
  SoftsecReceiveSa *rsa;
  SoftsecPeer *peer = NULL;
  MacsecAddress peer_address;
  unsigned peer_portid = 0;
  const unsigned char *dmac, *smac, *secdata;
  unsigned type, tci_an, sl;
  unsigned srchdrlen, dsthdrlen, datalen, confoff;
  unsigned vbit, esbit, scbit, scbbit, ebit, cbit;
  unsigned char *u8p;
  uint32_t pnlo, u32;
  uint64_t u64;

  id->intf = intf;
  id->dstbuf = dstbuf;
  id->srcbuf = srcbuf;
  id->srclen = srclen;

  /* use real port until a virtual port, if any, is identified */
  port = &intf->rport;
  id->portid = 1;

  /* get ethernet header */
  if (!get_pointer(&b, &dmac, 6) ||
      !get_pointer(&b, &smac, 6) ||
      !get_uint16(&b, &type))
    {
      port->pstat.if_in_errors++;
      goto drop;
    }

  /* handle untagged frames */
  if (type != 0x88e5)
    {
      if (port->ctrl.validate_frames == MACSEC_VALIDATE_FRAMES_STRICT)
        {
          port->stat.in_pkts_no_tag++;
          port->pstat.if_in_discards++;
          goto drop;
        }

      if (dstmax < srclen)
        {
          port->pstat.if_in_discards++;
          goto drop;
        }

      memcpy(dstbuf, srcbuf, srclen);
      id->dstlen = srclen;
      port->stat.in_pkts_untagged++;
      id->state = SOFTSEC_INPUT_PASS;
      return id->state;
    }

  /* get SecTAG fields between ethertype and SCI */
  if (!get_uint8(&b, &tci_an) ||
      !get_uint8(&b, &sl) ||
      !get_uint32(&b, &pnlo))
    goto badtag;

  /* break out TCI/AN */
  vbit = (tci_an >> 7) & 1;
  esbit = (tci_an >> 6) & 1;
  scbit = (tci_an >> 5) & 1;
  scbbit = (tci_an >> 4) & 1;
  ebit = (tci_an >> 3) & 1;
  cbit = (tci_an >> 2) & 1;
  id->an = tci_an & 3;

  /* drop nonzero version, bad ES/SC, SCB=1 or bad short length */
  if (vbit != 0 || (scbit & (esbit | scbbit)) || (sl && sl >= 48))
    goto badtag;

  /* get explicit or end system SCI */
  if (scbit)
    {
      if (!get_buffer(&b, peer_address.buf, 6) ||
          !get_uint16(&b, &peer_portid))
        goto badtag;
    }
  else if (esbit || scbbit)
    {
      memcpy(peer_address.buf, smac, 6);
      peer_portid = 1;
    }

  /* mark end of header */
  srchdrlen = b.buf - srcbuf;

  /* get secure data and ICV, observing potential short length */
  if (sl)
    {
      if (!get_pointer(&b, &secdata, sl))
        goto badtag;
      datalen = sl;
      if (!get_pointer(&b, &id->icv, 16))
        goto badtag;
    }
  else
    {
      if (!get_pointer_tail(&b, &id->icv, 16))
        goto badtag;
      secdata = b.buf;
      datalen = b.len;
    }

  /* silently drop C=0, E=1 */
  if (!cbit && ebit)
    {
      port->pstat.if_in_errors++;
      goto drop;
    }

  /* find peer object */
  if (scbit || esbit || scbbit)
    {
      /* use SCI from the wire */
      peer = softsec_peers_lookup(&intf->peers, &peer_address);
    }
  else
    {
      /* no SCI, try point-to-point case */
      if (softsec_peers_count(&intf->peers) == 1)
        {
          peer = softsec_peers_first(&intf->peers);
          peer_address = peer->peer_address;
          peer_portid = peer->peer_portid;
        }
      else
        {
          /* zero or multiple peers, cannot determine peer SCI */
          peer = NULL;
        }
    }

  /* check if we have a matching peer or not */
  if (!peer || peer_portid != peer->peer_portid)
    {
      if (cbit || port->ctrl.validate_frames == MACSEC_VALIDATE_FRAMES_STRICT)
        {
          port->stat.in_pkts_no_sci++;
          port->pstat.if_in_errors++;
          goto drop;
        }
      else
        {
          port->stat.in_pkts_unknown_sci++;
          goto untag;
        }
    }

  /* get real or virtual port and set up some convenience flags */
  if (peer->portid > 1)
    port = &peer->vport;
  else
    port = &intf->rport;
  id->portid = peer->portid;
  id->strict = (port->ctrl.validate_frames == MACSEC_VALIDATE_FRAMES_STRICT);
  switch (port->ctrl.current_cipher_suite)
    {
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_256:
      id->xpn = 1;
      break;
    default:
      id->xpn = 0;
      break;
    }

  /* get receive SC and SA */
  rsc = &peer->receive_sc;
  rsa = &rsc->sas[id->an];

  /* drop or pass packets to inactive SA */
  if (!rsa->enable)
    {
      if (id->strict || cbit)
        {
          rsa->stat.in_pkts_not_using_sa++;
          rsc->stat.in_pkts_not_using_sa++;
          port->pstat.if_in_errors++;
          goto drop;
        }
      rsa->stat.in_pkts_unused_sa++;
      rsc->stat.in_pkts_unused_sa++;
      goto untag;
    }

  id->crypto_handle = rsa->dkey->crypto_handle;

  /* use 32-bit packet number or recover 64-bit one */
  if (!id->xpn)
    {
      id->pn = pnlo;
    }
  else
    {
      id->pn = (rsa->lowest_pn & 0xffffffff00000000ULL) | pnlo;
      if (((uint32_t)(rsa->lowest_pn) & 0x80000000) && !(pnlo & 0x80000000))
        id->pn += 0x0000000100000000ULL;
    }

  /* replay protection */
  if (port->ctrl.replay_protect && id->pn < rsa->lowest_pn)
    {
      rsc->stat.in_pkts_late++;
      port->pstat.if_in_discards++;
      goto drop;
    }

  /* copy MAC addresses to destination buffer */
  if (dstmax < 12 + datalen)
    {
      port->pstat.if_in_discards++;
      goto drop;
    }
  memcpy(dstbuf, dmac, 6);
  memcpy(dstbuf + 6, smac, 6);
  dsthdrlen = 12;

  /* special procedure if cryptographic validation is disabled */
  if (port->ctrl.validate_frames == MACSEC_VALIDATE_FRAMES_DISABLED)
    {
      /* drop if data transformed */
      if (cbit)
        {
          rsa->stat.in_pkts_not_valid++;
          rsc->stat.in_pkts_not_valid++;
          port->pstat.if_in_errors++;
          goto drop;
        }

      /* count delayed frame (in non-antireplay mode), otherwise unchecked */
      if (id->pn < rsa->lowest_pn)
        rsc->stat.in_pkts_delayed++;
      else
        rsc->stat.in_pkts_unchecked++;

      /* pass decapsulated frame */
      memcpy(dstbuf + dsthdrlen, secdata, datalen);
      id->dstlen = dsthdrlen + datalen;
      id->state = SOFTSEC_INPUT_PASS;
      return id->state;
    }

  /* validate frame */
  if (!id->xpn)
    {
      /* create 96-bit IV from SCI and PN */
      memcpy(id->iv, peer_address.buf, 6);
      if (!scbbit)
        {
          id->iv[6] = (unsigned char)(peer_portid >> 8);
          id->iv[7] = (unsigned char)peer_portid;
        }
      else
        {
          id->iv[6] = 0;
          id->iv[7] = 0;
        }
      memcpy(id->iv + 8, srcbuf + 16, 4);
    }
  else
    {
      /* create 96-bit IV from SSCI, salt and 64-bit PN */
      u32 = rsa->ssci ^ rsa->dkey->salthi;
      u8p = (void *)&u64;
      *u8p++ = ((id->pn & 0xff00000000000000ULL) >> 56);
      *u8p++ = ((id->pn & 0x00ff000000000000ULL) >> 48);
      *u8p++ = ((id->pn & 0x0000ff0000000000ULL) >> 40);
      *u8p++ = ((id->pn & 0x000000ff00000000ULL) >> 32);
      *u8p++ = ((id->pn & 0x00000000ff000000ULL) >> 24);
      *u8p++ = ((id->pn & 0x0000000000ff0000ULL) >> 16);
      *u8p++ = ((id->pn & 0x000000000000ff00ULL) >> 8);
      *u8p++ = ((id->pn & 0x00000000000000ffULL) >> 0);
      u64 ^= rsa->dkey->saltlo;
      memcpy(id->iv, &u32, 4);
      memcpy(id->iv + 4, &u64, 8);
    }

  /* determine length of and copy integrity-only user data */
  switch (port->ctrl.confidentiality_offset)
    {
    default:
    case  MACSEC_CONFIDENTIALITY_OFFSET_0:
      confoff = 0;
      break;
    case  MACSEC_CONFIDENTIALITY_OFFSET_30:
      confoff = 30;
      break;
    case  MACSEC_CONFIDENTIALITY_OFFSET_50:
      confoff = 50;
      break;
    case  MACSEC_CONFIDENTIALITY_NOT_USED:
      confoff = datalen;
      break;
    }
  if (!ebit || confoff > datalen)
    confoff = datalen;
  memcpy(dstbuf + dsthdrlen, srcbuf + srchdrlen, confoff);

  id->srcauth = srchdrlen + confoff;
  id->dstskip = dsthdrlen + confoff;
  id->decrlen = datalen - confoff;

  id->rsc = rsc;
  id->rsa = rsa;
  id->ebit = ebit;
  id->cbit = cbit;
  id->state = SOFTSEC_INPUT_DECRYPT;
  return id->state;

 untag:
  /* copy addresses and secure data to result buffer */
  if (dstmax < 12 + datalen)
    {
      port->pstat.if_in_discards++;
      goto drop;
    }
  memcpy(dstbuf, dmac, 6);
  memcpy(dstbuf + 6, smac, 6);
  memcpy(dstbuf + 12, secdata, datalen);
  id->dstlen = 12 + datalen;
  id->state = SOFTSEC_INPUT_PASS;
  return id->state;

 badtag:
  port->stat.in_pkts_bad_tag++;
  port->pstat.if_in_errors++;
 drop:
  id->state = SOFTSEC_INPUT_DROP;
  return id->state;
}

unsigned
softsec_input_end(
  SoftsecInputDescriptor *id,
  unsigned *portid)
{
  ReadBuffer b = {id->srcbuf, id->srclen};
  SoftsecReceiveSc *rsc = id->rsc;
  SoftsecReceiveSa *rsa = id->rsa, *oldrsa;
  SoftsecPort *port = get_port(id->intf, id->portid);
  uint32_t replay_window;
  unsigned valid, dstlen;

  if (id->state == SOFTSEC_INPUT_PASS)
    {
      *portid = id->portid;
      dstlen = id->dstlen;
      goto pass2;
    }
  else if (id->state != SOFTSEC_INPUT_DECRYPT)
    return 0;

  /* update validation octet statistics */
  port->stat.in_octets_validated += id->srcauth;
  port->stat.in_octets_decrypted += id->decrlen;

  /* point to end of secure data */
  if (!get_skip(&b, id->srcauth + id->decrlen))
    {
      port->pstat.if_in_errors++;
      goto drop;
    }

  /* get ICV verification result */
  valid = id->authentic;

  /* drop or pass unsuccessfully validated frame */
  if (!valid)
    {
      /* drop if strict mode or frame transformed */
      if (id->strict || id->cbit)
        {
          rsa->stat.in_pkts_not_valid++;
          rsc->stat.in_pkts_not_valid++;
          port->pstat.if_in_errors++;
          goto drop;
        }
      /* otherwise pass (in check mode) */
      else
        {
          rsa->stat.in_pkts_invalid++;
          rsc->stat.in_pkts_invalid++;
          goto pass;
        }
    }

  /* pass delayed frame (in non-antireplay mode) */
  if (id->pn < rsa->lowest_pn)
    {
      rsc->stat.in_pkts_delayed++;
      goto pass;
    }

  /* at this point frame is cryptographically validated and can be
     used to update protocol state */

  /* turn off previous SA if it was active */
  if (rsc->oan != id->an)
    {
      oldrsa = &rsc->sas[rsc->oan];
      rsc->oan = id->an;
      if (oldrsa->enable)
        {
          oldrsa->enable = 0;
          oldrsa->stat.in_use = 0;
          oldrsa->stat.stopped_time = id->intf->sysuptime;
        }
    }

  /* update receive packet numbers */
  if (id->pn >= rsa->next_pn)
    {
      if ((!id->xpn && (uint32_t)rsa->next_pn >= 0xffffffff) ||
          (id->xpn && rsa->next_pn >= 0xffffffffffffffffULL))
        {
          rsa->enable = 0;
          rsa->stat.in_use = 0;
          rsa->stat.stopped_time = id->intf->sysuptime;
          rsc->stat.receiving = 0;
          rsc->stat.stopped_time = id->intf->sysuptime;
          update_controlled_mac(id->intf, id->portid);
        }
      else
        {
          rsa->next_pn = id->pn + 1;
          rsa->stat.next_pn = rsa->next_pn;
        }

      /* clamp replay window to less than 2^30 for XPN */
      replay_window = port->ctrl.replay_window;
      if (id->xpn)
        replay_window &=  ~0xc0000000;

      if (rsa->next_pn <= replay_window)
        rsa->lowest_pn = 1;
      else
        rsa->lowest_pn = rsa->next_pn - replay_window;
    }

  /* count it */
  rsa->stat.in_pkts_ok++;
  rsc->stat.in_pkts_ok++;

  /* succeed */
 pass:
  *portid = id->portid;
  dstlen = id->dstskip + id->decrlen;
 pass2:
  port->pstat.if_in_octets += dstlen;
  if (!(id->dstbuf[0] & 1))
    port->pstat.if_in_ucast_pkts++;
  else if (!memcmp(id->dstbuf, &broadcast_address, 6))
    port->pstat.if_in_broadcast_pkts++;
  else
    port->pstat.if_in_multicast_pkts++;
  return dstlen;

 drop:
  return 0;
}

void
softsec_get_encryption_parameters(
  SoftsecOutputDescriptor *od,
  void **crypto_handle,
  SoftsecCryptoWriteBytes *ci,
  SoftsecCryptoTag **tag,
  const SoftsecCryptoIv **iv,
  SoftsecCryptoReadBytes *pl,
  SoftsecCryptoReadBytes *aad)
{
  *crypto_handle = od->crypto_handle;
  ci->buf = od->dstbuf + od->dstauth;
  ci->len = od->encrlen;
  *tag = (void *)&od->icv;
  *iv = (void *)&od->iv;
  pl->buf = od->srcbuf + od->srcskip;
  pl->len = od->encrlen;
  aad->buf = od->dstbuf;
  aad->len = od->dstauth;
}

void
softsec_get_decryption_parameters(
  SoftsecInputDescriptor *id,
  void **crypto_handle,
  unsigned **authentic,
  SoftsecCryptoWriteBytes *pl,
  const SoftsecCryptoIv **iv,
  SoftsecCryptoReadBytes *ci,
  SoftsecCryptoReadBytes *aad,
  const SoftsecCryptoTag **tag)
{
  *crypto_handle = id->crypto_handle;
  *authentic = &id->authentic;
  pl->buf = id->dstbuf + id->dstskip;
  pl->len = id->decrlen;
  *iv = (void *)&id->iv;
  ci->buf = id->srcbuf + id->srcauth;
  ci->len = id->decrlen;
  aad->buf = id->srcbuf;
  aad->len = id->srcauth;
  *tag = (void *)id->icv;
}

#ifdef SOFTSEC_SOFTWARE_CRYPTO
unsigned
softsec_test(void)
{
  unsigned ok = 1;

  ok &= test_integrity_54();
  ok &= test_integrity_60();
  ok &= test_integrity_65();
  ok &= test_integrity_79();
  ok &= test_confidentiality_54();
  ok &= test_confidentiality_60();
  ok &= test_confidentiality_61();
  ok &= test_confidentiality_75();

  return ok;
}
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

/*
 * Static functions.
 */

static void
clear_peer(
  SoftsecPeer *peer)
{
#ifdef SOFTSEC_SOFTWARE_CRYPTO
  int i;

  for (i = 0; i < sizeof peer->vport.dkeys / sizeof peer->vport.dkeys[0]; i++)
    softsec_crypto_uninit(&peer->vport.dkeys[i].crypto);
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

  memset(peer, 0, sizeof *peer);
}

static void
init_secy_ctrl(MacsecSecyCtrl *ctrl)
{

  ctrl->validate_frames = MACSEC_VALIDATE_FRAMES_STRICT;
  ctrl->replay_protect = 1;
  ctrl->replay_window = 0;
  ctrl->protect_frames = 1;
  ctrl->always_include_sci = 0;
  ctrl->use_es = 0;
  ctrl->use_scb = 0;
  ctrl->current_cipher_suite = MACSEC_CIPHER_SUITE_GCM_AES_128;
  ctrl->confidentiality_offset = MACSEC_CONFIDENTIALITY_OFFSET_0;
  ctrl->admin_point_to_point_mac = MACSEC_ADMIN_POINT_TO_POINT_MAC_AUTO;
}

static void
update_controlled_mac(SoftsecIntf *intf, unsigned portid)
{
  SoftsecPort *port = get_port(intf, portid);
  SoftsecPeer *peer;
  unsigned macsec, transmitting, receiving;

  macsec =
    port->ctrl.protect_frames ||
    port->ctrl.validate_frames == MACSEC_VALIDATE_FRAMES_STRICT;

  transmitting = port->transmit_sc.stat.transmitting;

  peer = NULL;
  receiving = 0;
  while ((peer = softsec_peers_next(&intf->peers, peer)))
    if (peer->portid == portid && peer->receive_sc.stat.receiving)
      receiving++;

  port->pstat.mac_enabled =
    port->controlled_port_enabled &&
    intf->mac_enabled &&
    (!macsec || (transmitting && receiving >= 1));

  port->pstat.mac_operational =
    port->pstat.mac_enabled &&
    intf->mac_operational;

  switch (port->ctrl.admin_point_to_point_mac)
    {
    case MACSEC_ADMIN_POINT_TO_POINT_MAC_FORCE_TRUE:
      port->pstat.oper_point_to_point_mac = 1;
      break;

    case MACSEC_ADMIN_POINT_TO_POINT_MAC_FORCE_FALSE:
      port->pstat.oper_point_to_point_mac = 0;
      break;

    case MACSEC_ADMIN_POINT_TO_POINT_MAC_AUTO:
    default:
      port->pstat.oper_point_to_point_mac =
        (port->ctrl.validate_frames == MACSEC_VALIDATE_FRAMES_STRICT &&
         receiving <= 1) ||
        (port->ctrl.validate_frames != MACSEC_VALIDATE_FRAMES_STRICT &&
         intf->oper_point_to_point_mac);
      break;
    }
}

static SoftsecPort *
get_port(const SoftsecIntf *intf, unsigned portid)
{
  SoftsecPeer *peer;

  if (portid > 1)
    {
      /* map large portid to real port for selftest purposes */
      if (portid >= sizeof intf->vport_peer / sizeof intf->vport_peer[0])
        return (void *)&intf->rport;
      peer = intf->vport_peer[portid];
      assert(peer != NULL);
      return &peer->vport;
    }
  else
    {
      return (void *)&intf->rport;
    }
}

static inline int
put_uint8(WriteBuffer *b, unsigned value)
{
  if (b->len < 1)
    return 0;

  *b->buf++ = (unsigned char )value;
  b->len--;

  return 1;
}

static inline int
get_uint8(ReadBuffer *b, unsigned *value)
{
  if (b->len < 1)
    return 0;

  *value = *b->buf++;
  b->len--;

  return 1;
}

static inline int
put_uint16(WriteBuffer *b, unsigned value)
{
  if (b->len < 2)
    return 0;

  *b->buf++ = (unsigned char)(value >> 8);
  *b->buf++ = (unsigned char)value;
  b->len -= 2;

  return 1;
}

static inline int
get_uint16(ReadBuffer *b, unsigned *value)
{
  if (b->len < 2)
    return 0;

  *value = *b->buf++ << 8;
  *value |= *b->buf++;
  b->len -= 2;

  return 1;
}

static inline int
put_uint32(WriteBuffer *b, uint32_t value)
{
  if (b->len < 4)
    return 0;

  *b->buf++ = (unsigned char)(value >> 24);
  *b->buf++ = (unsigned char)(value >> 16);
  *b->buf++ = (unsigned char)(value >> 8);
  *b->buf++ = (unsigned char)value;
  b->len -= 4;

  return 1;
}

static inline int
get_uint32(ReadBuffer *b, uint32_t *value)
{
  if (b->len < 4)
    return 0;

  *value = *b->buf++ << 24;
  *value |= *b->buf++ << 16;
  *value |= *b->buf++ << 8;
  *value |= *b->buf++;
  b->len -= 4;

  return 1;
}

static inline int
put_buffer(WriteBuffer *b, const unsigned char *buf, unsigned len)
{
  if (b->len < len)
    return 0;

  memcpy(b->buf, buf, len);
  b->buf += len;
  b->len -= len;

  return 1;
}

static inline int
get_buffer(ReadBuffer *b, unsigned char *buf, unsigned len)
{
  if (b->len < len)
    return 0;

  memcpy(buf, b->buf, len);
  b->buf += len;
  b->len -= len;

  return 1;
}

static inline int
put_space(WriteBuffer *b, unsigned char **ptr, unsigned len)
{
  if (b->len < len)
    return 0;

  *ptr = b->buf;
  b->buf += len;
  b->len -= len;

  return 1;
}

static inline int
put_skip(WriteBuffer *b, unsigned len)
{
  if (b->len < len)
    return 0;

  b->buf += len;
  b->len -= len;
  return 1;
}

static inline int
get_pointer(ReadBuffer *b, const unsigned char **ptr, unsigned len)
{
  if (b->len < len)
    return 0;

  *ptr = b->buf;
  b->buf += len;
  b->len -= len;

  return 1;
}

static inline int
get_pointer_tail(
  ReadBuffer *b, const unsigned char **ptr, unsigned len)
{
  if (b->len < len)
    return 0;

  *ptr = b->buf + b->len;
  *ptr -= len;
  b->len -= len;

  return 1;
}

static inline int
get_skip(ReadBuffer *b, unsigned len)
{
  if (b->len < len)
    return 0;

  b->buf += len;
  b->len -= len;
  return 1;
}

#ifdef SOFTSEC_SOFTWARE_CRYPTO
static unsigned
test_integrity_54(void)
{
  unsigned ok = 1;

  const unsigned char unprotected[] = {
    0xD6, 0x09, 0xB1, 0xF0, 0x56, 0x63,
    0x7A, 0x0D, 0x46, 0xDF, 0x99, 0x8D,
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x00, 0x01};
  const unsigned char header[] = {
    0xD6, 0x09, 0xB1, 0xF0, 0x56, 0x63,
    0x7A, 0x0D, 0x46, 0xDF, 0x99, 0x8D,
    0x88, 0xE5,
    0x22,
    0x2A,
    0xB2, 0xC2, 0x84, 0x65,
    0x12, 0x15, 0x35, 0x24, 0xC0, 0x89, 0x5E, 0x81};
  const unsigned char key128[] = {
    0xAD, 0x7A, 0x2B, 0xD0, 0x3E, 0xAC, 0x83, 0x5A,
    0x6F, 0x62, 0x0F, 0xDC, 0xB5, 0x06, 0xB3, 0x45};
  const unsigned char key256[] = {
    0xE3, 0xC0, 0x8A, 0x8F, 0x06, 0xC6, 0xE3, 0xAD,
    0x95, 0xA7, 0x05, 0x57, 0xB2, 0x3F, 0x75, 0x48,
    0x3C, 0xE3, 0x30, 0x21, 0xA9, 0xC7, 0x2B, 0x70,
    0x25, 0x66, 0x62, 0x04, 0xC6, 0x9C, 0x0B, 0x72,};
  const unsigned char icv128[] = {
    0xF0, 0x94, 0x78, 0xA9, 0xB0, 0x90, 0x07, 0xD0,
    0x6F, 0x46, 0xE9, 0xB6, 0xA1, 0xDA, 0x25, 0xDD};
  const unsigned char icv256[] = {
    0x2F, 0x0B, 0xC5, 0xAF, 0x40, 0x9E, 0x06, 0xD6,
    0x09, 0xEA, 0x8B, 0x7D, 0x0F, 0xA5, 0xEA, 0x50};

  const unsigned char xpn_ssci[] = {
    0x7A, 0x30, 0xC1, 0x18};
  const unsigned char xpn_pnhi[] = {
    0xB0, 0xDF, 0x45, 0x9C};
  const unsigned char xpn_salt[] = {
    0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2,
    0x1C, 0x66, 0xFA, 0x6D};
  const unsigned char xpn_icv128[] = {
    0x17, 0xFE, 0x19, 0x81, 0xEB, 0xDD, 0x4A, 0xFC,
    0x50, 0x62, 0x69, 0x7E, 0x8B, 0xAA, 0x0C, 0x23};
  const unsigned char xpn_icv256[] = {
    0x4D, 0xBD, 0x2F, 0x6A, 0x75, 0x4A, 0x6C, 0xF7,
    0x28, 0xCC, 0x12, 0x9B, 0xA6, 0x93, 0x15, 0x77};

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){icv128, sizeof icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){icv256, sizeof icv256});

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){xpn_icv128, sizeof xpn_icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){xpn_icv256, sizeof xpn_icv256});

  return ok;
}

static unsigned
test_integrity_60(void)
{
  unsigned ok = 1;

  const unsigned char unprotected[] = {
    0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D,
    0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D,
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x00, 0x03};
  const unsigned char header[] = {
    0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D,
    0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D,
    0x88, 0xE5,
    0x40,
    0x00,
    0x76, 0xD4, 0x57, 0xED};
  const unsigned char key128[] = {
    0x07, 0x1B, 0x11, 0x3B, 0x0C, 0xA7, 0x43, 0xFE,
    0xCC, 0xCF, 0x3D, 0x05, 0x1F, 0x73, 0x73, 0x82};
  const unsigned char icv128[] = {
    0x0C, 0x01, 0x7B, 0xC7, 0x3B, 0x22, 0x7D, 0xFC,
    0xC9, 0xBA, 0xFA, 0x1C, 0x41, 0xAC, 0xC3, 0x53};
  const unsigned char key256[] = {
    0x69, 0x1D, 0x3E, 0xE9, 0x09, 0xD7, 0xF5, 0x41,
    0x67, 0xFD, 0x1C, 0xA0, 0xB5, 0xD7, 0x69, 0x08,
    0x1F, 0x2B, 0xDE, 0x1A, 0xEE, 0x65, 0x5F, 0xDB,
    0xAB, 0x80, 0xBD, 0x52, 0x95, 0xAE, 0x6B, 0xE7};
  const unsigned char icv256[] = {
    0x35, 0x21, 0x7C, 0x77, 0x4B, 0xBC, 0x31, 0xB6,
    0x31, 0x66, 0xBC, 0xF9, 0xD4, 0xAB, 0xED, 0x07};

  const unsigned char xpn_ssci[] = {
    0x7A, 0x30, 0xC1, 0x18};
  const unsigned char xpn_pnhi[] = {
    0xB0, 0xDF, 0x45, 0x9C};
  const unsigned char xpn_salt[] = {
    0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2,
    0x1C, 0x66, 0xFA, 0x6D};
  const unsigned char xpn_icv128[] = {
    0xAB, 0xC4, 0x06, 0x85, 0xA3, 0xCF, 0x91, 0x1D,
    0x37, 0x87, 0xE4, 0x9D, 0xB6, 0xA7, 0x26, 0x5E};
  const unsigned char xpn_icv256[] = {
    0xAC, 0x21, 0x95, 0x7B, 0x83, 0x12, 0xAB, 0x3C,
    0x99, 0xAB, 0x46, 0x84, 0x98, 0x79, 0xC3, 0xF3};

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){icv128, sizeof icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){icv256, sizeof icv256});

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){xpn_icv128, sizeof xpn_icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){xpn_icv256, sizeof xpn_icv256});

  return ok;
}

static unsigned
test_integrity_65(void)
{
  unsigned ok = 1;

  const unsigned char unprotected[] = {
    0x84, 0xC5, 0xD5, 0x13, 0xD2, 0xAA,
    0xF6, 0xE5, 0xBB, 0xD2, 0x72, 0x77,
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
    0x3D, 0x3E, 0x3F, 0x00, 0x05};
  const unsigned char header[] = {
    0x84, 0xC5, 0xD5, 0x13, 0xD2, 0xAA,
    0xF6, 0xE5, 0xBB, 0xD2, 0x72, 0x77,
    0x88, 0xE5,
    0x23,
    0x00,
    0x89, 0x32, 0xD6, 0x12,
    0x7C, 0xFD, 0xE9, 0xF9, 0xE3, 0x37, 0x24, 0xC6};
  const unsigned char key128[] = {
    0x01, 0x3F, 0xE0, 0x0B, 0x5F, 0x11, 0xBE, 0x7F,
    0x86, 0x6D, 0x0C, 0xBB, 0xC5, 0x5A, 0x7A, 0x90};
  const unsigned char icv128[] = {
    0x21, 0x78, 0x67, 0xE5, 0x0C, 0x2D, 0xAD, 0x74,
    0xC2, 0x8C, 0x3B, 0x50, 0xAB, 0xDF, 0x69, 0x5A};
  const unsigned char key256[] = {
    0x83, 0xC0, 0x93, 0xB5, 0x8D, 0xE7, 0xFF, 0xE1,
    0xC0, 0xDA, 0x92, 0x6A, 0xC4, 0x3F, 0xB3, 0x60,
    0x9A, 0xC1, 0xC8, 0x0F, 0xEE, 0x1B, 0x62, 0x44,
    0x97, 0xEF, 0x94, 0x2E, 0x2F, 0x79, 0xA8, 0x23};
  const unsigned char icv256[] = {
    0x6E, 0xE1, 0x60, 0xE8, 0xFA, 0xEC, 0xA4, 0xB3,
    0x6C, 0x86, 0xB2, 0x34, 0x92, 0x0C, 0xA9, 0x75};

  const unsigned char xpn_ssci[] = {
    0x7A, 0x30, 0xC1, 0x18};
  const unsigned char xpn_pnhi[] = {
    0xB0, 0xDF, 0x45, 0x9C};
  const unsigned char xpn_salt[] = {
    0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2,
    0x1C, 0x66, 0xFA, 0x6D};
  const unsigned char xpn_icv128[] = {
    0x67, 0x85, 0x59, 0xB7, 0xE5, 0x2D, 0xB0, 0x06,
    0x82, 0xE3, 0xB8, 0x30, 0x34, 0xCE, 0xBE, 0x59};
  const unsigned char xpn_icv256[] = {
    0x84, 0xBA, 0xC8, 0xE5, 0x3D, 0x1E, 0xA3, 0x55,
    0xA5, 0xC7, 0xD3, 0x34, 0x84, 0x0A, 0xE9, 0x62};

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){icv128, sizeof icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){icv256, sizeof icv256});

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){xpn_icv128, sizeof xpn_icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){xpn_icv256, sizeof xpn_icv256});

  return ok;
}

static unsigned
test_integrity_79(void)
{
  unsigned ok = 1;

  const unsigned char unprotected[] = {
    0x68, 0xF2, 0xE7, 0x76, 0x96, 0xCE,
    0x7A, 0xE8, 0xE2, 0xCA, 0x4E, 0xC5,
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
    0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C,
    0x4D, 0x00, 0x07};
  const unsigned char header[] = {
    0x68, 0xF2, 0xE7, 0x76, 0x96, 0xCE,
    0x7A, 0xE8, 0xE2, 0xCA, 0x4E, 0xC5,
    0x88, 0xE5,
    0x41,
    0x00,
    0x2E, 0x58, 0x49, 0x5C};
  const unsigned char key128[] = {
    0x88, 0xEE, 0x08, 0x7F, 0xD9, 0x5D, 0xA9, 0xFB,
    0xF6, 0x72, 0x5A, 0xA9, 0xD7, 0x57, 0xB0, 0xCD};
  const unsigned char icv128[] = {
    0x07, 0x92, 0x2B, 0x8E, 0xBC, 0xF1, 0x0B, 0xB2,
    0x29, 0x75, 0x88, 0xCA, 0x4C, 0x61, 0x45, 0x23};
  const unsigned char key256[] = {
    0x4C, 0x97, 0x3D, 0xBC, 0x73, 0x64, 0x62, 0x16,
    0x74, 0xF8, 0xB5, 0xB8, 0x9E, 0x5C, 0x15, 0x51,
    0x1F, 0xCE, 0xD9, 0x21, 0x64, 0x90, 0xFB, 0x1C,
    0x1A, 0x2C, 0xAA, 0x0F, 0xFE, 0x04, 0x07, 0xE5};
  const unsigned char icv256[] = {
    0x00, 0xBD, 0xA1, 0xB7, 0xE8, 0x76, 0x08, 0xBC,
    0xBF, 0x47, 0x0F, 0x12, 0x15, 0x7F, 0x4C, 0x07};

  const unsigned char xpn_ssci[] = {
    0x7A, 0x30, 0xC1, 0x18};
  const unsigned char xpn_pnhi[] = {
    0xB0, 0xDF, 0x45, 0x9C};
  const unsigned char xpn_salt[] = {
    0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2,
    0x1C, 0x66, 0xFA, 0x6D};
  const unsigned char xpn_icv128[] = {
    0xD0, 0xDC, 0x89, 0x6D, 0xC8, 0x37, 0x98, 0xA7,
    0x9F, 0x3C, 0x5A, 0x95, 0xBA, 0x3C, 0xDF, 0x9A};
  const unsigned char xpn_icv256[] = {
    0x04, 0x24, 0x9A, 0x20, 0x8A, 0x65, 0xB9, 0x6B,
    0x3F, 0x32, 0x63, 0x00, 0x4C, 0xFD, 0x86, 0x7D};

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){icv128, sizeof icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){icv256, sizeof icv256});

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){xpn_icv128, sizeof xpn_icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){unprotected + 12, sizeof unprotected - 12},
    (TestVector){xpn_icv256, sizeof xpn_icv256});

  return ok;
}

static unsigned
test_confidentiality_54(void)
{
  unsigned ok = 1;

  const unsigned char unprotected[] = {
    0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D,
    0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D,
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x00, 0x04};
  const unsigned char header[] = {
    0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D,
    0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D,
    0x88, 0xE5,
    0x4C,
    0x2A,
    0x76, 0xD4, 0x57, 0xED};
  const unsigned char key128[] = {
    0x07, 0x1B, 0x11, 0x3B, 0x0C, 0xA7, 0x43, 0xFE,
    0xCC, 0xCF, 0x3D, 0x05, 0x1F, 0x73, 0x73, 0x82};
  const unsigned char secure128[] = {
    0x13, 0xB4, 0xC7, 0x2B, 0x38, 0x9D, 0xC5, 0x01,
    0x8E, 0x72, 0xA1, 0x71, 0xDD, 0x85, 0xA5, 0xD3,
    0x75, 0x22, 0x74, 0xD3, 0xA0, 0x19, 0xFB, 0xCA,
    0xED, 0x09, 0xA4, 0x25, 0xCD, 0x9B, 0x2E, 0x1C,
    0x9B, 0x72, 0xEE, 0xE7, 0xC9, 0xDE, 0x7D, 0x52,
    0xB3, 0xF3};
  const unsigned char icv128[] = {
    0xD6, 0xA5, 0x28, 0x4F, 0x4A, 0x6D, 0x3F, 0xE2,
    0x2A, 0x5D, 0x6C, 0x2B, 0x96, 0x04, 0x94, 0xC3};
  const unsigned char key256[] = {
    0x69, 0x1D, 0x3E, 0xE9, 0x09, 0xD7, 0xF5, 0x41,
    0x67, 0xFD, 0x1C, 0xA0, 0xB5, 0xD7, 0x69, 0x08,
    0x1F, 0x2B, 0xDE, 0x1A, 0xEE, 0x65, 0x5F, 0xDB,
    0xAB, 0x80, 0xBD, 0x52, 0x95, 0xAE, 0x6B, 0xE7};
  const unsigned char secure256[] = {
    0xC1, 0x62, 0x3F, 0x55, 0x73, 0x0C, 0x93, 0x53,
    0x30, 0x97, 0xAD, 0xDA, 0xD2, 0x56, 0x64, 0x96,
    0x61, 0x25, 0x35, 0x2B, 0x43, 0xAD, 0xAC, 0xBD,
    0x61, 0xC5, 0xEF, 0x3A, 0xC9, 0x0B, 0x5B, 0xEE,
    0x92, 0x9C, 0xE4, 0x63, 0x0E, 0xA7, 0x9F, 0x6C,
    0xE5, 0x19};
  const unsigned char icv256[] = {
    0x12, 0xAF, 0x39, 0xC2, 0xD1, 0xFD, 0xC2, 0x05,
    0x1F, 0x8B, 0x7B, 0x3C, 0x9D, 0x39, 0x7E, 0xF2};

  const unsigned char xpn_ssci[] = {
    0x7A, 0x30, 0xC1, 0x18};
  const unsigned char xpn_pnhi[] = {
    0xB0, 0xDF, 0x45, 0x9C};
  const unsigned char xpn_salt[] = {
    0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2,
    0x1C, 0x66, 0xFA, 0x6D};
  const unsigned char xpn_secure128[] = {
    0x9C, 0xA4, 0x69, 0x84, 0x43, 0x02, 0x03, 0xED,
    0x41, 0x6E, 0xBD, 0xC2, 0xFE, 0x26, 0x22, 0xBA,
    0x3E, 0x5E, 0xAB, 0x69, 0x61, 0xC3, 0x63, 0x83,
    0x00, 0x9E, 0x18, 0x7E, 0x9B, 0x0C, 0x88, 0x56,
    0x46, 0x53, 0xB9, 0xAB, 0xD2, 0x16, 0x44, 0x1C,
    0x6A, 0xB6};
  const unsigned char xpn_icv128[] = {
    0xF0, 0xA2, 0x32, 0xE9, 0xE4, 0x4C, 0x97, 0x8C,
    0xF7, 0xCD, 0x84, 0xD4, 0x34, 0x84, 0xD1, 0x01};
  const unsigned char xpn_secure256[] = {
    0x88, 0xD9, 0xF7, 0xD1, 0xF1, 0x57, 0x8E, 0xE3,
    0x4B, 0xA7, 0xB1, 0xAB, 0xC8, 0x98, 0x93, 0xEF,
    0x1D, 0x33, 0x98, 0xC9, 0xF1, 0xDD, 0x3E, 0x47,
    0xFB, 0xD8, 0x55, 0x3E, 0x0F, 0xF7, 0x86, 0xEF,
    0x56, 0x99, 0xEB, 0x01, 0xEA, 0x10, 0x42, 0x0D,
    0x0E, 0xBD};
  const unsigned char xpn_icv256[] = {
    0x39, 0xA0, 0xE2, 0x73, 0xC4, 0xC7, 0xF9, 0x5E,
    0xD8, 0x43, 0x20, 0x7D, 0x7A, 0x49, 0x7D, 0xFA};

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){secure128, sizeof secure128},
    (TestVector){icv128, sizeof icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){secure256, sizeof secure256},
    (TestVector){icv256, sizeof icv256});

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){xpn_secure128, sizeof xpn_secure128},
    (TestVector){xpn_icv128, sizeof xpn_icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){xpn_secure256, sizeof xpn_secure256},
    (TestVector){xpn_icv256, sizeof xpn_icv256});

  return ok;
}

static unsigned
test_confidentiality_60(void)
{
  unsigned ok = 1;

  const unsigned char unprotected[] = {
    0xD6, 0x09, 0xB1, 0xF0, 0x56, 0x63,
    0x7A, 0x0D, 0x46, 0xDF, 0x99, 0x8D,
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x00, 0x02};
  const unsigned char header[] = {
    0xD6, 0x09, 0xB1, 0xF0, 0x56, 0x63,
    0x7A, 0x0D, 0x46, 0xDF, 0x99, 0x8D,
    0x88, 0xE5,
    0x2E,
    0x00,
    0xB2, 0xC2, 0x84, 0x65,
    0x12, 0x15, 0x35, 0x24, 0xC0, 0x89, 0x5E, 0x81};
  const unsigned char key128[] = {
    0xAD, 0x7A, 0x2B, 0xD0, 0x3E, 0xAC, 0x83, 0x5A,
    0x6F, 0x62, 0x0F, 0xDC, 0xB5, 0x06, 0xB3, 0x45};
  const unsigned char secure128[] = {
    0x70, 0x1A, 0xFA, 0x1C, 0xC0, 0x39, 0xC0, 0xD7,
    0x65, 0x12, 0x8A, 0x66, 0x5D, 0xAB, 0x69, 0x24,
    0x38, 0x99, 0xBF, 0x73, 0x18, 0xCC, 0xDC, 0x81,
    0xC9, 0x93, 0x1D, 0xA1, 0x7F, 0xBE, 0x8E, 0xDD,
    0x7D, 0x17, 0xCB, 0x8B, 0x4C, 0x26, 0xFC, 0x81,
    0xE3, 0x28, 0x4F, 0x2B, 0x7F, 0xBA, 0x71, 0x3D};
  const unsigned char icv128[] = {
    0x4F, 0x8D, 0x55, 0xE7, 0xD3, 0xF0, 0x6F, 0xD5,
    0xA1, 0x3C, 0x0C, 0x29, 0xB9, 0xD5, 0xB8, 0x80};
  const unsigned char key256[] = {
    0xE3, 0xC0, 0x8A, 0x8F, 0x06, 0xC6, 0xE3, 0xAD,
    0x95, 0xA7, 0x05, 0x57, 0xB2, 0x3F, 0x75, 0x48,
    0x3C, 0xE3, 0x30, 0x21, 0xA9, 0xC7, 0x2B, 0x70,
    0x25, 0x66, 0x62, 0x04, 0xC6, 0x9C, 0x0B, 0x72};
  const unsigned char secure256[] = {
    0xE2, 0x00, 0x6E, 0xB4, 0x2F, 0x52, 0x77, 0x02,
    0x2D, 0x9B, 0x19, 0x92, 0x5B, 0xC4, 0x19, 0xD7,
    0xA5, 0x92, 0x66, 0x6C, 0x92, 0x5F, 0xE2, 0xEF,
    0x71, 0x8E, 0xB4, 0xE3, 0x08, 0xEF, 0xEA, 0xA7,
    0xC5, 0x27, 0x3B, 0x39, 0x41, 0x18, 0x86, 0x0A,
    0x5B, 0xE2, 0xA9, 0x7F, 0x56, 0xAB, 0x78, 0x36};
  const unsigned char icv256[] = {
    0x5C, 0xA5, 0x97, 0xCD, 0xBB, 0x3E, 0xDB, 0x8D,
    0x1A, 0x11, 0x51, 0xEA, 0x0A, 0xF7, 0xB4, 0x36};

  const unsigned char xpn_ssci[] = {
    0x7A, 0x30, 0xC1, 0x18};
  const unsigned char xpn_pnhi[] = {
    0xB0, 0xDF, 0x45, 0x9C};
  const unsigned char xpn_salt[] = {
    0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2,
    0x1C, 0x66, 0xFA, 0x6D};
  const unsigned char xpn_secure128[] = {
    0x07, 0x12, 0xD9, 0x80, 0xCA, 0x50, 0xBB, 0xED,
    0x35, 0xA0, 0xFA, 0x56, 0x63, 0x38, 0x72, 0x9F,
    0xFA, 0x16, 0xD1, 0x9F, 0xFC, 0xF0, 0x7B, 0x3A,
    0x1E, 0x79, 0x19, 0xB3, 0x77, 0x6A, 0xAC, 0xEC,
    0x8A, 0x59, 0x37, 0x20, 0x8B, 0x48, 0x3A, 0x76,
    0x91, 0x98, 0x4D, 0x38, 0x07, 0x92, 0xE0, 0x7F};
  const unsigned char xpn_icv128[] = {
    0xC2, 0xC3, 0xC7, 0x9F, 0x26, 0x3F, 0xA6, 0xBF,
    0xF8, 0xE7, 0x58, 0x1E, 0x2C, 0xE4, 0x5A, 0xF8};
  const unsigned char xpn_secure256[] = {
    0x3E, 0xB0, 0x4A, 0x4B, 0xBF, 0x54, 0xC6, 0xEB,
    0x12, 0x22, 0xA9, 0xAE, 0xA0, 0x0C, 0x38, 0x68,
    0x7F, 0x6C, 0x35, 0x20, 0xD9, 0x76, 0xA3, 0xB6,
    0x94, 0x80, 0x06, 0x50, 0xCE, 0x65, 0x85, 0xE6,
    0x20, 0xA4, 0x19, 0x19, 0x17, 0xD2, 0xA6, 0x05,
    0xD8, 0x70, 0xC7, 0x8D, 0x27, 0x52, 0xCE, 0x49};
  const unsigned char xpn_icv256[] = {
    0x3B, 0x44, 0x2A, 0xC0, 0xC8, 0x16, 0xD7, 0xAB,
    0xD7, 0x0A, 0xD6, 0x5C, 0x25, 0xD4, 0x64, 0x13};

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){secure128, sizeof secure128},
    (TestVector){icv128, sizeof icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){secure256, sizeof secure256},
    (TestVector){icv256, sizeof icv256});

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){xpn_secure128, sizeof xpn_secure128},
    (TestVector){xpn_icv128, sizeof xpn_icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){xpn_secure256, sizeof xpn_secure256},
    (TestVector){xpn_icv256, sizeof xpn_icv256});

  return ok;
}

static unsigned
test_confidentiality_61(void)
{
  unsigned ok = 1;

  const unsigned char unprotected[] = {
    0x84, 0xC5, 0xD5, 0x13, 0xD2, 0xAA,
    0xF6, 0xE5, 0xBB, 0xD2, 0x72, 0x77,
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x00,
    0x06};
  const unsigned char header[] = {
    0x84, 0xC5, 0xD5, 0x13, 0xD2, 0xAA,
    0xF6, 0xE5, 0xBB, 0xD2, 0x72, 0x77,
    0x88, 0xE5,
    0x2F,
    0x00,
    0x89, 0x32, 0xD6, 0x12,
    0x7C, 0xFD, 0xE9, 0xF9, 0xE3, 0x37, 0x24, 0xC6};
  const unsigned char key128[] = {
    0x01, 0x3F, 0xE0, 0x0B, 0x5F, 0x11, 0xBE, 0x7F,
    0x86, 0x6D, 0x0C, 0xBB, 0xC5, 0x5A, 0x7A, 0x90};
  const unsigned char secure128[] = {
    0x3A, 0x4D, 0xE6, 0xFA, 0x32, 0x19, 0x10, 0x14,
    0xDB, 0xB3, 0x03, 0xD9, 0x2E, 0xE3, 0xA9, 0xE8,
    0xA1, 0xB5, 0x99, 0xC1, 0x4D, 0x22, 0xFB, 0x08,
    0x00, 0x96, 0xE1, 0x38, 0x11, 0x81, 0x6A, 0x3C,
    0x9C, 0x9B, 0xCF, 0x7C, 0x1B, 0x9B, 0x96, 0xDA,
    0x80, 0x92, 0x04, 0xE2, 0x9D, 0x0E, 0x2A, 0x76,
    0x42};
  const unsigned char icv128[] = {
    0xBF, 0xD3, 0x10, 0xA4, 0x83, 0x7C, 0x81, 0x6C,
    0xCF, 0xA5, 0xAC, 0x23, 0xAB, 0x00, 0x39, 0x88};
  const unsigned char key256[] = {
    0x83, 0xC0, 0x93, 0xB5, 0x8D, 0xE7, 0xFF, 0xE1,
    0xC0, 0xDA, 0x92, 0x6A, 0xC4, 0x3F, 0xB3, 0x60,
    0x9A, 0xC1, 0xC8, 0x0F, 0xEE, 0x1B, 0x62, 0x44,
    0x97, 0xEF, 0x94, 0x2E, 0x2F, 0x79, 0xA8, 0x23};
  const unsigned char secure256[] = {
    0x11, 0x02, 0x22, 0xFF, 0x80, 0x50, 0xCB, 0xEC,
    0xE6, 0x6A, 0x81, 0x3A, 0xD0, 0x9A, 0x73, 0xED,
    0x7A, 0x9A, 0x08, 0x9C, 0x10, 0x6B, 0x95, 0x93,
    0x89, 0x16, 0x8E, 0xD6, 0xE8, 0x69, 0x8E, 0xA9,
    0x02, 0xEB, 0x12, 0x77, 0xDB, 0xEC, 0x2E, 0x68,
    0xE4, 0x73, 0x15, 0x5A, 0x15, 0xA7, 0xDA, 0xEE,
    0xD4};
  const unsigned char icv256[] = {
    0xA1, 0x0F, 0x4E, 0x05, 0x13, 0x9C, 0x23, 0xDF,
    0x00, 0xB3, 0xAA, 0xDC, 0x71, 0xF0, 0x59, 0x6A};

  const unsigned char xpn_ssci[] = {
    0x7A, 0x30, 0xC1, 0x18};
  const unsigned char xpn_pnhi[] = {
    0xB0, 0xDF, 0x45, 0x9C};
  const unsigned char xpn_salt[] = {
    0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2,
    0x1C, 0x66, 0xFA, 0x6D};
  const unsigned char xpn_secure128[] = {
    0x14, 0xC1, 0x76, 0x93, 0xBC, 0x82, 0x97, 0xEE,
    0x6C, 0x47, 0xC5, 0x65, 0xCB, 0xE0, 0x67, 0x9E,
    0x80, 0xF0, 0x0F, 0xCA, 0xF5, 0x92, 0xC9, 0xAA,
    0x04, 0x73, 0x92, 0x8E, 0x7F, 0x2F, 0x21, 0x6F,
    0xF5, 0xA0, 0x33, 0xDE, 0xC7, 0x51, 0x3F, 0x45,
    0xD3, 0x4C, 0xBB, 0x98, 0x1C, 0x5B, 0xD6, 0x4E,
    0x8B};
  const unsigned char xpn_icv128[] = {
    0xD8, 0x4B, 0x8E, 0x2A, 0x78, 0xE7, 0x4D, 0xAF,
    0xEA, 0xA0, 0x38, 0x46, 0xFE, 0x93, 0x0C, 0x0E};
  const unsigned char xpn_secure256[] = {
    0x09, 0x96, 0xE0, 0xC9, 0xA5, 0x57, 0x74, 0xE0,
    0xA7, 0x92, 0x30, 0x4E, 0x7D, 0xC1, 0x50, 0xBD,
    0x67, 0xFD, 0x74, 0x7D, 0xD1, 0xB9, 0x41, 0x95,
    0x94, 0xBF, 0x37, 0x3D, 0x4A, 0xCE, 0x8F, 0x87,
    0xF5, 0xC1, 0x34, 0x9A, 0xFA, 0xC4, 0x91, 0xAA,
    0x0A, 0x40, 0xD3, 0x19, 0x90, 0x87, 0xB2, 0x9F,
    0xDF};
  const unsigned char xpn_icv256[] = {
    0x80, 0x2F, 0x05, 0x0E, 0x69, 0x1F, 0x11, 0xA2,
    0xD9, 0xB3, 0x58, 0xF6, 0x99, 0x41, 0x84, 0xF5};

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){secure128, sizeof secure128},
    (TestVector){icv128, sizeof icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){secure256, sizeof secure256},
    (TestVector){icv256, sizeof icv256});

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){xpn_secure128, sizeof xpn_secure128},
    (TestVector){xpn_icv128, sizeof xpn_icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){xpn_secure256, sizeof xpn_secure256},
    (TestVector){xpn_icv256, sizeof xpn_icv256});

  return ok;
}

static unsigned
test_confidentiality_75(void)
{
  unsigned ok = 1;

  const unsigned char unprotected[] = {
    0x68, 0xF2, 0xE7, 0x76, 0x96, 0xCE,
    0x7A, 0xE8, 0xE2, 0xCA, 0x4E, 0xC5,
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
    0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x00, 0x08};
  const unsigned char header[] = {
    0x68, 0xF2, 0xE7, 0x76, 0x96, 0xCE,
    0x7A, 0xE8, 0xE2, 0xCA, 0x4E, 0xC5,
    0x88, 0xE5,
    0x4D,
    0x00,
    0x2E, 0x58, 0x49, 0x5C};
  const unsigned char key128[] = {
    0x88, 0xEE, 0x08, 0x7F, 0xD9, 0x5D, 0xA9, 0xFB,
    0xF6, 0x72, 0x5A, 0xA9, 0xD7, 0x57, 0xB0, 0xCD};
  const unsigned char secure128[] = {
    0xC3, 0x1F, 0x53, 0xD9, 0x9E, 0x56, 0x87, 0xF7,
    0x36, 0x51, 0x19, 0xB8, 0x32, 0xD2, 0xAA, 0xE7,
    0x07, 0x41, 0xD5, 0x93, 0xF1, 0xF9, 0xE2, 0xAB,
    0x34, 0x55, 0x77, 0x9B, 0x07, 0x8E, 0xB8, 0xFE,
    0xAC, 0xDF, 0xEC, 0x1F, 0x8E, 0x3E, 0x52, 0x77,
    0xF8, 0x18, 0x0B, 0x43, 0x36, 0x1F, 0x65, 0x12,
    0xAD, 0xB1, 0x6D, 0x2E, 0x38, 0x54, 0x8A, 0x2C,
    0x71, 0x9D, 0xBA, 0x72, 0x28, 0xD8, 0x40};
  const unsigned char icv128[] = {
    0x88, 0xF8, 0x75, 0x7A, 0xDB, 0x8A, 0xA7, 0x88,
    0xD8, 0xF6, 0x5A, 0xD6, 0x68, 0xBE, 0x70, 0xE7};
  const unsigned char key256[] = {
    0x4C, 0x97, 0x3D, 0xBC, 0x73, 0x64, 0x62, 0x16,
    0x74, 0xF8, 0xB5, 0xB8, 0x9E, 0x5C, 0x15, 0x51,
    0x1F, 0xCE, 0xD9, 0x21, 0x64, 0x90, 0xFB, 0x1C,
    0x1A, 0x2C, 0xAA, 0x0F, 0xFE, 0x04, 0x07, 0xE5};
  const unsigned char secure256[] = {
    0xBA, 0x8A, 0xE3, 0x1B, 0xC5, 0x06, 0x48, 0x6D,
    0x68, 0x73, 0xE4, 0xFC, 0xE4, 0x60, 0xE7, 0xDC,
    0x57, 0x59, 0x1F, 0xF0, 0x06, 0x11, 0xF3, 0x1C,
    0x38, 0x34, 0xFE, 0x1C, 0x04, 0xAD, 0x80, 0xB6,
    0x68, 0x03, 0xAF, 0xCF, 0x5B, 0x27, 0xE6, 0x33,
    0x3F, 0xA6, 0x7C, 0x99, 0xDA, 0x47, 0xC2, 0xF0,
    0xCE, 0xD6, 0x8D, 0x53, 0x1B, 0xD7, 0x41, 0xA9,
    0x43, 0xCF, 0xF7, 0xA6, 0x71, 0x3B, 0xD0};
  const unsigned char icv256[] = {
    0x26, 0x11, 0xCD, 0x7D, 0xAA, 0x01, 0xD6, 0x1C,
    0x5C, 0x88, 0x6D, 0xC1, 0xA8, 0x17, 0x01, 0x07};

  const unsigned char xpn_ssci[] = {
    0x7A, 0x30, 0xC1, 0x18};
  const unsigned char xpn_pnhi[] = {
    0xB0, 0xDF, 0x45, 0x9C};
  const unsigned char xpn_salt[] = {
    0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2,
    0x1C, 0x66, 0xFA, 0x6D};
  const unsigned char xpn_secure128[] = {
    0xEA, 0xEC, 0xC6, 0xAF, 0x65, 0x12, 0xFC, 0x8B,
    0x6C, 0x8C, 0x43, 0xBC, 0x55, 0xB1, 0x90, 0xB2,
    0x62, 0x6D, 0x07, 0xD3, 0xD2, 0x18, 0xFA, 0xF5,
    0xDA, 0xA7, 0xD8, 0xF8, 0x00, 0xA5, 0x73, 0x31,
    0xEB, 0x43, 0xB5, 0xA1, 0x7A, 0x37, 0xE5, 0xB1,
    0xD6, 0x0D, 0x27, 0x5C, 0xCA, 0xF7, 0xAC, 0xD7,
    0x04, 0xCC, 0x9A, 0xCE, 0x2B, 0xF8, 0xBC, 0x8B,
    0x9B, 0x23, 0xB9, 0xAD, 0xF0, 0x2F, 0x87};
  const unsigned char xpn_icv128[] = {
    0x34, 0x6B, 0x96, 0xD1, 0x13, 0x6A, 0x75, 0x4D,
    0xF0, 0xA6, 0xCD, 0xE1, 0x26, 0xC1, 0x07, 0xF8};
  const unsigned char xpn_secure256[] = {
    0xB0, 0xFE, 0xA3, 0x63, 0x18, 0xB9, 0xB3, 0x64,
    0x66, 0xC4, 0x6E, 0x9E, 0x1B, 0xDA, 0x1A, 0x26,
    0x68, 0x58, 0x19, 0x6E, 0x7E, 0x70, 0xD8, 0x82,
    0xAE, 0x70, 0x47, 0x56, 0x68, 0xCD, 0xE4, 0xEC,
    0x88, 0x3F, 0x6A, 0xC2, 0x36, 0x9F, 0x28, 0x4B,
    0xED, 0x1F, 0xE3, 0x2F, 0x42, 0x09, 0x2F, 0xDF,
    0xF5, 0x86, 0x8A, 0x3C, 0x64, 0xE5, 0x61, 0x51,
    0x92, 0xA7, 0xA3, 0x76, 0x0B, 0x34, 0xBC};
  const unsigned char xpn_icv256[] = {
    0x85, 0x69, 0x2C, 0xD8, 0x15, 0xB6, 0x64, 0x71,
    0x1A, 0xEF, 0x91, 0x1D, 0xF7, 0x8D, 0x7F, 0x46};

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){secure128, sizeof secure128},
    (TestVector){icv128, sizeof icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){NULL, 0},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){secure256, sizeof secure256},
    (TestVector){icv256, sizeof icv256});

  ok &= test_bidirectional(
    (TestVector){key128, sizeof key128},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){xpn_secure128, sizeof xpn_secure128},
    (TestVector){xpn_icv128, sizeof xpn_icv128});

  ok &= test_bidirectional(
    (TestVector){key256, sizeof key256},
    (TestVector){xpn_ssci, sizeof xpn_ssci},
    (TestVector){xpn_pnhi, sizeof xpn_pnhi},
    (TestVector){xpn_salt, sizeof xpn_salt},
    (TestVector){unprotected, sizeof unprotected},
    (TestVector){header, sizeof header},
    (TestVector){xpn_secure256, sizeof xpn_secure256},
    (TestVector){xpn_icv256, sizeof xpn_icv256});

  return ok;
}

static unsigned
test_bidirectional(
  TestVector key,
  TestVector ssci,
  TestVector pnhi,
  TestVector salt,
  TestVector unprotected,
  TestVector header,
  TestVector secure,
  TestVector icv)
{
  SoftsecIntf intf;
  MacsecKey sak;
  MacsecKi ki;
  SoftsecDkey *dkey;
  unsigned an, portid, peer_portid;
  uint64_t next_pn, lowest_pn;
  SoftsecPeer *peer;
  MacsecAddress *address, *peer_address;
  MacsecSalt *saltp;
  MacsecSsci *sscip;
  unsigned char protected[128], dstbuf[128];
  unsigned proteclen, dstlen;
  unsigned explicit_sci, confidentiality;
  unsigned ok = 1;

  assert(sizeof protected >= header.len + secure.len + icv.len);
  memcpy(protected, header.buf, header.len);
  proteclen = header.len;
  memcpy(protected + proteclen, secure.buf, secure.len);
  proteclen += secure.len;
  memcpy(protected + proteclen, icv.buf, icv.len);
  proteclen += icv.len;

  assert(header.len == 20 || header.len == 28);
  if (header.len == 20)
    explicit_sci = 0;
  else
    explicit_sci = 1;

  confidentiality = !!(header.buf[14] & 0x08);

  if (explicit_sci)
    {
      address = (void *)&protected[20];
      portid = (protected[26] << 8) | protected[27];
    }
  else
    {
      address = (void *)&protected[6];
      portid = 1;
    }

  softsec_init_intf(&intf, address);

  if (key.len == 16 && ssci.len == 0)
    {
      intf.rport.ctrl.current_cipher_suite =
        MACSEC_CIPHER_SUITE_GCM_AES_128;
    }
  else if (key.len == 32 && ssci.len == 0)
    {
      intf.rport.ctrl.current_cipher_suite =
        MACSEC_CIPHER_SUITE_GCM_AES_256;
    }
  else if (key.len == 16 && ssci.len != 0)
    {
      intf.rport.ctrl.current_cipher_suite =
        MACSEC_CIPHER_SUITE_GCM_AES_XPN_128;
    }
  else if (key.len == 32 && ssci.len != 0)
    {
      intf.rport.ctrl.current_cipher_suite =
        MACSEC_CIPHER_SUITE_GCM_AES_XPN_256;
    }
  else
    {
      assert(0);
    }

  if (confidentiality)
    intf.rport.ctrl.confidentiality_offset =
      MACSEC_CONFIDENTIALITY_OFFSET_0;
  else
    intf.rport.ctrl.confidentiality_offset =
      MACSEC_CONFIDENTIALITY_NOT_USED;

  if (explicit_sci)
    intf.rport.ctrl.always_include_sci = 1;
  else
    intf.rport.ctrl.use_es = 1;

  peer_address = address;
  peer_portid = portid;
  peer = softsec_create_peer(&intf, 1, peer_address, peer_portid);

  if (ssci.len == 0)
    {
      saltp = NULL;
      sscip = NULL;
    }
  else
    {
      assert(salt.len == sizeof *saltp);
      saltp = (void *)salt.buf;
      assert(ssci.len == sizeof *sscip);
      sscip = (void *)ssci.buf;
    }

  memcpy(sak.buf, key.buf, key.len);
  sak.len = key.len;
  memset(&ki, 0, sizeof ki);
  dkey = softsec_install_key(&intf, 1, &sak, &ki, saltp);

  an = protected[14] & 3;
  next_pn =
    (uint32_t)(
      (protected[16] << 24) | (protected[17] << 16) |
      (protected[18] << 8) | protected[19]);

  if (ssci.len != 0)
    {
      assert(pnhi.len == 4);
      next_pn |= (uint64_t)pnhi.buf[0] << 56;
      next_pn |= (uint64_t)pnhi.buf[1] << 48;
      next_pn |= (uint64_t)pnhi.buf[2] << 40;
      next_pn |= (uint64_t)pnhi.buf[3] << 32;
    }

  lowest_pn = next_pn;

  softsec_create_transmit_sa(&intf, 1, an, next_pn, dkey, sscip);
  softsec_enable_transmit(&intf, 1, an);
  softsec_create_receive_sa(peer, an, lowest_pn, dkey, sscip);
  softsec_enable_receive(peer, an);

  assert(sizeof dstbuf >= proteclen);

  /* use invalid portid on purpose and rely on get_port() mapping
     it to the real port */
  dstlen = softsec_output(
    &intf, portid, dstbuf, sizeof dstbuf, unprotected.buf, unprotected.len);
  ok &= (dstlen == proteclen && !memcmp(dstbuf, protected, dstlen));

  dstlen = softsec_input(
    &intf, &portid, dstbuf, sizeof dstbuf, protected, proteclen);
  ok &= (dstlen == unprotected.len && !memcmp(dstbuf, unprotected.buf,dstlen));

  return ok;
}
#endif /* SOFTSEC_SOFTWARE_CRYPTO */
