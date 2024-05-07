/**

   802.1X-2010 internal utility functions.

   File: dot1xpae_util.c

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#include "dot1xpae_util.h"
#include "dot1xpae_eapol.h"
#include "macseclmi.h"
#include <assert.h>
#include <gtOs/gtOsInit.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedMemory.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*
 * Prototypes.
 */

/*
 * Recalculate SSCIs for the local station and for each peer station
 * based on the temporary solution in 802.1AEbw draft 1.1: station
 * with highest SCI gets 0x00000001, next highest 0x00000002 and so
 * on. As the MAC addresses of stations are unique and form the most
 * significant parts of SCIs, this is equivalent on using MAC
 * addresses instead of SCIs.
 */
static void
recalc_sscis(Dot1xIntf *intf);

/*
 * Store 32-bit unsigned in network byte order.
 */
static void
put32(unsigned char *buf, uint32_t n);

/*
 * EAPOL group addresses.
 */
static const MacsecAddress eapol_address_bridge ={
  .buf = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00}};
static const MacsecAddress eapol_address_pae = {
  .buf = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03}};
static const MacsecAddress eapol_address_lldp = {
  .buf = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e}};

/*
 * Cipher suite reference numbers.
 */
static const unsigned char cipher_suite_gcm_aes_128[8] = {
  0x00, 0x80, 0xc2, 0x00, 0x01, 0x00, 0x00, 0x01};
static const unsigned char cipher_suite_gcm_aes_256[8] = {
  0x00, 0x80, 0xc2, 0x00, 0x01, 0x00, 0x00, 0x02};
static const unsigned char cipher_suite_gcm_aes_xpn_128[8] = {
  0x00, 0x80, 0xc2, 0x00, 0x01, 0x00, 0x00, 0x03};
static const unsigned char cipher_suite_gcm_aes_xpn_256[8] = {
  0x00, 0x80, 0xc2, 0x00, 0x01, 0x00, 0x00, 0x04};

/*
 * Public functions.
 */

Dot1xNetwork *
dot1x_choose_network(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;

  if (intf->snetwork)
    return intf->snetwork;
  else if (port->rnetwork)
    return port->rnetwork;
  else if (port->cnetwork)
    return port->cnetwork;
  else
    return &intf->dnetwork;
}

void
dot1x_foreach_port(Dot1xIntf *intf, void (*func)(Dot1xPort *port))
{
  Dot1xPeer *peer, *next;

  func(&intf->rport);

  next = dot1x_peers_next(&intf->peers, NULL);
  while ((peer = next))
  {
    next = dot1x_peers_next(&intf->peers, peer);
    if (peer->port == &peer->vport)
      func(&peer->vport);
  }
}

Dot1xPort *
dot1x_refer_port(Dot1xIntf *intf, const MacsecAddress *peer_address)
{
  Dot1xPeer *peer;

  if (!intf->ctrl.virtual_ports_enable || !peer_address)
    return &intf->rport;

  if (!(peer = dot1x_refer_peer(intf, DOT1X_PEER_VIRTUAL, peer_address)))
    return NULL;

  return &peer->vport;
}

void
dot1x_unref_port(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;
  Dot1xPeer *peer;

  if (port == &intf->rport)
    return;

  peer = (void *)((unsigned char *)port - offsetof(Dot1xPeer, vport));
  dot1x_unref_peer(intf, peer);
}

void
dot1x_reref_port(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;
  Dot1xPeer *peer;

  if (port == &intf->rport)
    return;

  peer = (void *)((unsigned char *)port - offsetof(Dot1xPeer, vport));
  peer->reference_count++;
}

static int
dot1x_get_virtual_port_id(
        Dot1xIntf *intf)
{
  int portid;
  int loop;

  for (loop = 0; loop < 0xffff; loop++)
   {
     Dot1xPeer *next = NULL;

     portid = intf->next_vportid++;
     if (portid == 0xffff)
       intf->next_vportid = 2;

     while ((next = dot1x_peers_next(&intf->peers, next)))
       {
         if (next->port && next->port->portid == portid)
           break;
       }

     if (next == NULL)
       break;
   }

  return portid;
}


Dot1xPeer *
dot1x_refer_peer_portid(
  Dot1xIntf *intf,
  Dot1xPeerType type,
  const MacsecAddress *peer_address,
  int portid)
{
  Dot1xPeer *peer;
  Dot1xPort *port;
  Dot1xFormatBuffer fb;

  /* try finding an existing peer by MAC address */
  if (!(peer = dot1x_peers_lookup(&intf->peers, peer_address)))
    {
      /* no existing peer, try creating a new one */
      if (!(peer = dot1x_peers_insert(&intf->peers, peer_address)))
        {
          dot1x_error(intf, "out of peer entries");
          return NULL;
        }

      if (type == DOT1X_PEER_REAL)
        {
          /* associate peer with the real port */
          port = &intf->rport;
          peer->port = port;
        }
      else
        {
          if (portid < 0)
            {
              portid = dot1x_get_virtual_port_id(intf);
            }
          /* associate peer with the port object contained in the peer
             object itself and set a virtual port identifier */
          port = &peer->vport;
          peer->port = port;
          port->intf = intf;
          port->portid = portid;

          /* initialize any nonzero fields in the port object */
          macsec_list_init(&port->key_sources);
        }

      peer->has_sa = 0;

      /* recalculate XPN SSCIs after peer list change */
      recalc_sscis(intf);

      dot1x_debug_peer(
        peer, "created with MAC address %s",
        dot1x_format_address(&fb, &peer->peer_address));
    }
  peer->reference_count++;
  return peer;
}

Dot1xPeer *
dot1x_refer_peer(
        Dot1xIntf *intf, Dot1xPeerType type, const MacsecAddress *peer_address)
{
    return dot1x_refer_peer_portid(intf, type, peer_address, -1);
}

void
dot1x_unref_peer(Dot1xIntf *intf, Dot1xPeer *peer)
{
  if (--peer->reference_count <= 0)
    {
      dot1x_debug_peer(peer, "being destroyed");

      /* remove peer from list and return it to a zero-initialized
         state */
      assert(dot1x_peers_remove(&intf->peers, &peer->peer_address) ==peer);
      memset(peer, 0, sizeof *peer);

      /* recalculate XPN SSCIs after peer list change */
      recalc_sscis(intf);
    }
}

void
dot1x_reref_peer(Dot1xIntf *intf, Dot1xPeer *peer)
{
  (void)intf;
  peer->reference_count++;
}

void
dot1x_derive_key(
  Dot1xIntf *intf, unsigned char *buf, unsigned len,
  const MacsecKey *key, const char *label,
  void (*context)(void *arg, Dot1xCryptoCmac *cmac), void *arg)
{
  Dot1xCryptoCmac *cmac = &intf->crypto_cmac;
  unsigned char b, length[2];
  int n, i;

  assert(len == 16 || len == 32);

  /* number of iterations each producing 16 bytes of output */
  if (len == 16)
    n = 1;
  else
    n = 2;

  /* big-endian output length in bits */
  length[0] = (len << 3) >> 8;
  length[1] = (len << 3) & 0xff;

  for (i = 0; i < n; i++)
    {
      dot1x_crypto_cmac_start(cmac, key);

      b = (unsigned char)(i + 1);
      dot1x_crypto_cmac_update(cmac, &b, 1);
      dot1x_crypto_cmac_update(cmac, (unsigned char *)label, strlen(label));
      b = 0x00;
      dot1x_crypto_cmac_update(cmac, &b, 1);
      context(arg, cmac);
      dot1x_crypto_cmac_update(cmac, length, 2);
      dot1x_crypto_cmac_finish(cmac, (void *)(buf + 16 * i)); /* XXX cast */
    }
}

const MacsecAddress *
dot1x_eapol_address(Dot1xEapolGroup group)
{
  switch (group)
    {
    case DOT1X_EAPOL_GROUP_BRIDGE:
      return &eapol_address_bridge;
    default:
    case DOT1X_EAPOL_GROUP_PAE:
      return &eapol_address_pae;
    case DOT1X_EAPOL_GROUP_LLDP:
      return &eapol_address_lldp;
    }
}

void
dot1x_print(Dot1xIntf *intf, int category, const char *fmt, ...)
{
  va_list ap;

  if (!dot1x_debug_enabled(intf, category))
    return;

  va_start(ap, fmt);
  dot1x_vprintf(intf, fmt, ap);
  dot1x_printf(intf, "\n");
  va_end(ap);
}

void
dot1x_print_network(Dot1xNetwork *network, int category, const char *fmt, ...)
{
  va_list ap;

  if (!dot1x_debug_enabled(network->intf, category))
    return;

  va_start(ap, fmt);
  if (network == &network->intf->dnetwork)
    dot1x_printf(network->intf, "default network ");
  else
    dot1x_printf(
      network->intf, "network #%u ",
      (unsigned)(network - network->intf->networks.elemv));
  dot1x_vprintf(network->intf, fmt, ap);
  dot1x_printf(network->intf, "\n");
  va_end(ap);
}

void
dot1x_print_port(Dot1xPort *port, int category, const char *fmt, ...)
{
  va_list ap;

  if (!dot1x_debug_enabled(port->intf, category))
    return;

  va_start(ap, fmt);
  dot1x_printf(port->intf, "port %u ", port->portid);
  dot1x_vprintf(port->intf, fmt, ap);
  dot1x_printf(port->intf, "\n");
  va_end(ap);
}

void
dot1x_print_peer(
  Dot1xPeer *peer, int category, const char *fmt, ...)
{
  va_list ap;

  if (!dot1x_debug_enabled(peer->port->intf, category))
    return;

  va_start(ap, fmt);
  dot1x_printf(
    peer->port->intf, "peer #%u ",
    (unsigned)(peer - peer->port->intf->peers.elemv));
  dot1x_vprintf(peer->port->intf, fmt, ap);
  dot1x_printf(peer->port->intf, "\n");
  va_end(ap);
}

void
dot1x_print_mka(Dot1xMka *mka, int category, const char *fmt, ...)
{
  va_list ap;
  Dot1xIntf *intf = mka->cak->port->intf;

  if (!dot1x_debug_enabled(intf, category))
    return;

  va_start(ap, fmt);
  dot1x_printf(
    intf, "MKA actor #%u ", (unsigned)(mka - intf->mkas.datav));
  dot1x_vprintf(intf, fmt, ap);
  dot1x_printf(intf, "\n");
  va_end(ap);
}

void
dot1x_print_mka_peer(
  Dot1xMkaPeer *mkap, int category, const char *fmt, ...)
{
  va_list ap;
  Dot1xIntf *intf = mkap->mka->cak->port->intf;

  if (!dot1x_debug_enabled(intf, category))
    return;

  va_start(ap, fmt);
  dot1x_printf(
    intf, "MKA peer #%u/#%u ",
    (unsigned)(mkap->mka - intf->mkas.datav),
    (unsigned)(mkap - mkap->mka->peers.elemv));
  dot1x_vprintf(intf, fmt, ap);
  dot1x_printf(intf, "\n");
  va_end(ap);
}

void
dot1x_print_pacp(Dot1xPacp *pacp, int category, const char *fmt, ...)
{
  va_list ap;
  Dot1xIntf *intf = pacp->side->intf;

  if (!dot1x_debug_enabled(intf, category))
    return;

  va_start(ap, fmt);
  dot1x_printf(
    intf, "PACP machine #%u (%s) ",
    (unsigned)(pacp - pacp->side->pacps.elemv), dot1x_pacp_side(pacp->side));
  dot1x_vprintf(intf, fmt, ap);
  dot1x_printf(intf, "\n");
  va_end(ap);
}

void
dot1x_print_cak(Dot1xCak *cak, int category, const char *fmt, ...)
{
  va_list ap;

  if (!dot1x_debug_enabled(cak->intf, category))
    return;

  va_start(ap, fmt);
  dot1x_printf(
    cak->intf, "CAK #%u (%s) ",
    (unsigned)(cak - cak->intf->caks.elemv), dot1x_cak_type(cak->type));
  dot1x_vprintf(cak->intf, fmt, ap);
  dot1x_printf(cak->intf, "\n");
  va_end(ap);
}

void
dot1x_print_hex(Dot1xIntf *intf, const unsigned char *buf, unsigned len)
{
  int i, j;

  for (i = 0, j = 0; i < len; i++, j++, j &= 15)
    {
      if (j == 0)
        dot1x_printf(intf, "   ");

      dot1x_printf(intf, " %02x", buf[i]);

      if (j == 15)
        dot1x_printf(intf, "\n");
    }
  if (j != 15)
    dot1x_printf(intf, "\n");
}

void
dot1x_printf(Dot1xIntf *intf, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  dot1x_vprintf(intf, fmt, ap);
  va_end(ap);
}

void
dot1x_vprintf(Dot1xIntf *intf, const char *fmt, va_list ap)
{
  int n;
  char *cp;

  n = cpssOsVsnprintf(
    intf->message_buf + intf->message_end,
    sizeof intf->message_buf - intf->message_end,
    fmt, ap);

  if (n <= 0 || n >= (int)sizeof intf->message_buf - intf->message_end)
    {
      intf->message_buf[sizeof intf->message_buf - 1] = '\0';
      dot1x_platform_log(
        "%s: %s", intf->ifname, intf->message_buf + intf->message_start);
      intf->message_start = 0;
      intf->message_end = 0;
      return;
    }

  intf->message_end += n;

  if (!(cp = strchr(intf->message_buf + intf->message_start, '\n')))
    return;

  *cp++ = '\0';
  dot1x_platform_log(
          "%s: %s", intf->ifname, intf->message_buf + intf->message_start);
  intf->message_start = cp - intf->message_buf;

  if (intf->message_start >= intf->message_end)
    {
      intf->message_start = 0;
      intf->message_end = 0;
    }
}

const char *
dot1x_format_network(
  Dot1xFormatBuffer *fb, const Dot1xNetwork *netw)
{
  Dot1xFormatBuffer fb2;

  if (netw->nid.len <= 0)
    return "default network";
  else
    cpssOsSnprintf(
      *fb, sizeof *fb, "network %s", dot1x_format_nid(&fb2, &netw->nid));
  (*fb)[sizeof *fb - 1] = '\0';
  return *fb;
}

const char *
dot1x_format_pacp(Dot1xFormatBuffer *fb, const Dot1xPacp *pacp)
{
  cpssOsSnprintf(
    *fb, sizeof *fb, "PACP machine #%u",
    (unsigned)(pacp - pacp->side->pacps.elemv));
  (*fb)[sizeof *fb - 1] = '\0';
  return *fb;
}

const char *
dot1x_format_cak(Dot1xFormatBuffer *fb, const Dot1xCak *cak)
{
  cpssOsSnprintf(
    *fb, sizeof *fb, "CAK #%u", (unsigned)(cak - cak->intf->caks.elemv));
  (*fb)[sizeof *fb - 1] = '\0';
  return *fb;
}

const char *
dot1x_format_sci(
  Dot1xFormatBuffer *fb, const MacsecSci *sci)
{
  int fi;

  dot1x_format_address(fb, &sci->address);

  fi = strlen(*fb);

  cpssOsSnprintf(*fb + fi, sizeof *fb - fi, "/%u", sci->portid);
  (*fb)[sizeof *fb - 1] = '\0';
  return *fb;
}

const char *
dot1x_format_address(
  Dot1xFormatBuffer *fb, const MacsecAddress *address)
{
  return dot1x_format_hex(fb, address->buf, sizeof address->buf);
}

const char *
dot1x_format_ckn(
  Dot1xFormatBuffer *fb, const Dot1xCkn *ckn)
{
  return dot1x_format_buf(fb, ckn->buf, ckn->len);
}

const char *
dot1x_format_key(
  Dot1xFormatBuffer *fb, const MacsecKey *key)
{
  return dot1x_format_hex(fb, key->buf, key->len);
}

const char *
dot1x_format_wrap(
  Dot1xFormatBuffer *fb, const Dot1xCryptoWrap *wrap)
{
  return dot1x_format_hex(fb, wrap->buf, wrap->len);
}

const char *
dot1x_format_icv(
  Dot1xFormatBuffer *fb, const Dot1xIcv *icv)
{
  return dot1x_format_hex(fb, icv->buf, sizeof icv->buf);
}

const char *
dot1x_format_key_identifier(
  Dot1xFormatBuffer *fb, const Dot1xKeyIdentifier *ki)
{
  int fi;

  dot1x_format_member_identifier(fb, &ki->mi);

  fi = strlen(*fb);

  cpssOsSnprintf(
    *fb + fi, sizeof *fb - fi, "-%lu", (unsigned long int)ki->kn);
  (*fb)[sizeof *fb - 1] = '\0';
  return *fb;
}

const char *
dot1x_format_member_identifier(
  Dot1xFormatBuffer *fb, const Dot1xMemberIdentifier *mi)
{
  return dot1x_format_hex(fb, mi->buf, sizeof mi->buf);
}

const char *
dot1x_format_nid(
  Dot1xFormatBuffer *fb, const Dot1xNid *nid)
{
  return dot1x_format_utf8(fb, nid->buf, nid->len);
}

const char *
dot1x_format_kmd(
  Dot1xFormatBuffer *fb, const Dot1xKmd *kmd)
{
  return dot1x_format_utf8(fb, kmd->buf, kmd->len);
}

const char *
dot1x_format_utf8(
  Dot1xFormatBuffer *fb, const unsigned char *buf, unsigned len)
{
  int fi = 0, pos = 0, n, i;
  unsigned char b;
  unsigned u;
  char c;

  while (pos < len)
    {
      /* decode first byte of a character, set n to # of remaining bytes */
      b = buf[pos++];
      if ((b & 0x80) == 0x00)
        {
          u = b & 0x7f;
          n = 0;
        }
      else if ((b & 0xe0) == 0xc0)
        {
          u = b & 0x1f;
          n = 1;
        }
      else if ((b & 0xf0) == 0xe0)
        {
          u = b & 0x0f;
          n = 2;
        }
      else if ((b & 0xf8) == 0xf0)
        {
          u = b & 0x07;
          n = 3;
        }
      else
        {
          goto end; /* bad UTF-8 first byte */
        }

      /* decode remaining bytes of a character */
      for (i = 0; i < n; i++)
        {
          if (pos >= len)
            goto end; /* truncated UTF-8 character */
          b = buf[pos++];
          if ((b & 0xc0) != 0x80)
            goto end; /* bad UTF-8 byte */
          u <<= 6;
          u |= b & 0x3f;
        }

      if (u >= 0x20 && u < 0x7f)
        c = (char)u;
      else
        c = '_';

      if (fi + 1 >= sizeof *fb)
        goto end;

      (*fb)[fi++] = c;
    }

 end:
  (*fb)[fi] = '\0';
  return *fb;
}

const char *
dot1x_format_vprintf(Dot1xFormatBuffer *fb, const char *fmt, va_list ap)
{
  cpssOsVsnprintf(*fb, sizeof *fb, fmt, ap);
  (*fb)[sizeof *fb -1] = '\0';
  return *fb;
}

const char *
dot1x_format_buf(
  Dot1xFormatBuffer *fb, const unsigned char *buf, unsigned len)
{
  if (len <= 0)
    {
      strncpy(*fb, "<empty>", sizeof *fb);
      (*fb)[sizeof *fb - 1] = '\0';
      return *fb;
    }

  return dot1x_format_hex(fb, buf, len);
}

const char *
dot1x_format_hex(
  Dot1xFormatBuffer *fb, const unsigned char *buf, unsigned len)
{
  int fi = 0, i;

  for (i = 0; i < len; i++)
    {
      if (i > 0)
        {
          if (fi + 1 >= sizeof *fb)
            break;
          (*fb)[fi++] = ':';
        }

      if (fi + 2 >= sizeof *fb)
        break;
      cpssOsSnprintf(*fb + fi, 3, "%2.2X", (unsigned)buf[i]);
      fi += 2;
    }

  (*fb)[fi] = '\0';
  return *fb;
}

const char *
dot1x_pacp_side(Dot1xPacpSide *side)
{
  if (side == &side->intf->auth)
    return "authenticator";
  else if (side == &side->intf->supp)
    return "supplicant";
  else
    return "<unknown>";
}

const char *
dot1x_eapol_group(Dot1xEapolGroup n)
{
  switch (n)
    {
    case DOT1X_EAPOL_GROUP_BRIDGE:
      return "Bridge";
    case DOT1X_EAPOL_GROUP_PAE:
      return "PAE";
    case DOT1X_EAPOL_GROUP_LLDP:
      return "LLDP";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_pacp_state(int n)
{
  switch (n)
    {
    case DOT1X_PACP_BEGIN:
      return "BEGIN";
    case DOT1X_PACP_INITIALIZE:
      return "INITIALIZE";
    case DOT1X_PACP_LOGOFF:
      return "LOGOFF";
    case DOT1X_PACP_HELD:
      return "HELD";
    case DOT1X_PACP_UNAUTHENTICATED:
      return "UNAUTHENTICATED";
    case DOT1X_PACP_AUTHENTICATING:
      return "AUTHENTICATING";
    case DOT1X_PACP_AUTHENTICATED:
      return "AUTHENTICATED";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_cp_state(int n)
{
  switch (n)
    {
    case DOT1X_CP_BEGIN:
      return "BEGIN";
    case DOT1X_CP_INIT:
      return "INIT";
    case DOT1X_CP_CHANGE:
      return "CHANGE";
    case DOT1X_CP_ALLOWED:
      return "ALLOWED";
    case DOT1X_CP_AUTHENTICATED:
      return "AUTHENTICATED";
    case DOT1X_CP_SECURED:
      return "SECURED";
    case DOT1X_CP_RECEIVE:
      return "RECEIVE";
    case DOT1X_CP_RECEIVING:
      return "RECEIVING";
    case DOT1X_CP_READY:
      return "READY";
    case DOT1X_CP_TRANSMIT:
      return "TRANSMIT";
    case DOT1X_CP_ABANDON:
      return "ABANDON";
    case DOT1X_CP_TRANSMITTING:
      return "TRANSMITTING";
    case DOT1X_CP_RETIRE:
      return "RETIRE";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_connect(int n)
{
  switch (n)
    {
    case DOT1X_CONNECT_PENDING:
      return "PENDING";
    case DOT1X_CONNECT_UNAUTHENTICATED:
      return "UNAUTHENTICATED";
    case DOT1X_CONNECT_AUTHENTICATED:
      return "AUTHENTICATED";
    case DOT1X_CONNECT_SECURE:
      return "SECURE";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_use_eap(Dot1xUseEap n)
{
  switch (n)
    {
    case DOT1X_USE_EAP_NEVER:
      return "NEVER";
    case DOT1X_USE_EAP_IMMEDIATE:
      return "IMMEDIATE";
    case DOT1X_USE_EAP_MKA_FAIL:
      return "MKA_FAIL";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_unauth_allowed(Dot1xUnauthAllowed n)
{
  switch (n)
    {
    case DOT1X_UNAUTH_ALLOWED_NEVER:
      return "NEVER";
    case DOT1X_UNAUTH_ALLOWED_IMMEDIATE:
      return "IMMEDIATE";
    case DOT1X_UNAUTH_ALLOWED_AUTH_FAIL:
      return "AUTH_FAIL";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_unsecured_allowed(Dot1xUnsecuredAllowed n)
{
  switch (n)
    {
    case DOT1X_UNSECURED_ALLOWED_NEVER:
      return "NEVER";
    case DOT1X_UNSECURED_ALLOWED_IMMEDIATE:
      return "IMMEDIATE";
    case DOT1X_UNSECURED_ALLOWED_MKA_FAIL:
      return "MKA_FAIL";
    case DOT1X_UNSECURED_ALLOWED_MKA_SERVER:
      return "MKA_SERVER";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_access_status(Dot1xAccessStatus n)
{
  switch (n)
    {
    case DOT1X_ACCESS_STATUS_NO_ACCESS:
      return "no access";
    case DOT1X_ACCESS_STATUS_REMEDIAL_ACCESS:
      return "remedial access";
    case DOT1X_ACCESS_STATUS_RESTRICTED_ACCESS:
      return "restricted access";
    case DOT1X_ACCESS_STATUS_EXPECTED_ACCESS:
      return "expected access";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_unauthenticated_access(Dot1xUnauthenticatedAccess n)
{
  switch (n)
    {
    case DOT1X_UNAUTHENTICATED_ACCESS_NO_ACCESS:
      return "no access";
    case DOT1X_UNAUTHENTICATED_ACCESS_FALLBACK_ACCESS:
      return "fallback access";
    case DOT1X_UNAUTHENTICATED_ACCESS_LIMITED_ACCESS:
      return "limited access";
    case DOT1X_UNAUTHENTICATED_ACCESS_OPEN_ACCESS:
      return "open access";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_validate_frames(MacsecValidateFrames n)
{
  switch (n)
    {
    case MACSEC_VALIDATE_FRAMES_DISABLED:
      return "DISABLED";
    case MACSEC_VALIDATE_FRAMES_CHECK:
      return "CHECK";
    case MACSEC_VALIDATE_FRAMES_STRICT:
      return "STRICT";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_cipher_suite(MacsecCipherSuite n)
{
  switch (n)
    {
    case MACSEC_CIPHER_SUITE_GCM_AES_128:
      return "GCM-AES-128";
    case MACSEC_CIPHER_SUITE_GCM_AES_256:
      return "GCM-AES-256";
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
      return "GCM-AES-XPN-128";
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_256:
      return "GCM-AES-XPN-256";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_macsec_capability(MacsecCapability n)
{
  switch (n)
    {
    case MACSEC_CAPABILITY_NOT_IMPLEMENTED:
      return "not implemented";
    case MACSEC_CAPABILITY_INTEGRITY:
      return "integrity only";
    case MACSEC_CAPABILITY_CONFIDENTIALITY:
      return "integrity and confidentiality";
    case MACSEC_CAPABILITY_CONFIDENTIALITY_WITH_OFFSET:
      return "integrity and confidentiality with offsets";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_mka_algorithm(int n)
{
  switch (n)
    {
    case DOT1X_MKA_ALGORITHM_8021X_2009:
      return "802.1X-2009";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_confidentiality_offset(MacsecConfidentialityOffset n)
{
  switch (n)
    {
    case MACSEC_CONFIDENTIALITY_NOT_USED:
      return "confidentiality not used";
    case MACSEC_CONFIDENTIALITY_OFFSET_0:
      return "0";
    case MACSEC_CONFIDENTIALITY_OFFSET_30:
      return "30";
    case MACSEC_CONFIDENTIALITY_OFFSET_50:
      return "50";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_bool(int value)
{
  if (value)
    return "TRUE";
  else
    return "FALSE";
}

const char *
dot1x_cak_type(Dot1xCakType type)
{
  switch (type)
    {
    case DOT1X_CAK_GROUP:
      return "pre-shared group";
    case DOT1X_CAK_PAIRWISE:
      return "pre-shared pairwise";
    case DOT1X_CAK_EAP:
      return "EAP-based pairwise";
    case DOT1X_CAK_DISTRIBUTED:
      return "distributed group";
    default:
      return "<unknown>";
    }
}

const char *
dot1x_eapol_type(unsigned type)
{
  switch (type)
    {
    case DOT1X_EAPOL_START:
      return "Start";
    case DOT1X_EAPOL_LOGOFF:
      return "Logoff";
    case DOT1X_EAPOL_EAP:
      return "EAP";
    case DOT1X_EAPOL_MKA:
      return "MKA";
    case DOT1X_EAPOL_ANNOUNCEMENT_REQ:
      return "Announcement-Req";
    case DOT1X_EAPOL_ANNOUNCEMENT_GEN:
      return "Announcement (generic)";
    case DOT1X_EAPOL_ANNOUNCEMENT_SPC:
      return "Announcement (specific)";
    default:
      return "<unknown>";
    }
}

int
dot1x_get_cipher_suite_refnr(
  const unsigned char (**refnr)[8], MacsecCipherSuite cs)
{
  switch (cs)
    {
    case MACSEC_CIPHER_SUITE_GCM_AES_128:
      *refnr = &cipher_suite_gcm_aes_128;
      return 1;
    case MACSEC_CIPHER_SUITE_GCM_AES_256:
      *refnr = &cipher_suite_gcm_aes_256;
      return 1;
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
      *refnr = &cipher_suite_gcm_aes_xpn_128;
      return 1;
    case MACSEC_CIPHER_SUITE_GCM_AES_XPN_256:
      *refnr = &cipher_suite_gcm_aes_xpn_256;
      return 1;
    default:
      return 0;
    }
}

int
dot1x_get_cipher_suite_ident(MacsecCipherSuite *cs, unsigned char (*refnr)[8])
{
  if (!memcmp(*refnr, cipher_suite_gcm_aes_128, sizeof *refnr))
    {
      *cs = MACSEC_CIPHER_SUITE_GCM_AES_128;
      return 1;
    }
  else if (!memcmp(*refnr, cipher_suite_gcm_aes_256, sizeof *refnr))
    {
      *cs = MACSEC_CIPHER_SUITE_GCM_AES_256;
      return 1;
    }
  else if (!memcmp(*refnr, cipher_suite_gcm_aes_xpn_128, sizeof *refnr))
    {
      *cs = MACSEC_CIPHER_SUITE_GCM_AES_XPN_128;
      return 1;
    }
  else if (!memcmp(*refnr, cipher_suite_gcm_aes_xpn_256, sizeof *refnr))
    {
      *cs = MACSEC_CIPHER_SUITE_GCM_AES_XPN_256;
      return 1;
    }
  return 0;
}

/*
 * Static functions.
 */

static void
recalc_sscis(Dot1xIntf *intf)
{
  unsigned pos, n;
  Dot1xPeer *peer;
  uint32_t ssci;

  /* calc first (highest) SSCI for ascending MAC address traversal */
  ssci = dot1x_peers_count(&intf->peers) + 1;

  /* get the would-be position of the local MAC address in the sorted
     list of peer MAC addresses */
  dot1x_peers_bsearch(&intf->peers, &intf->local_address, &pos);

  /* traverse peers less than local station */
  peer = NULL;
  for (n = 0; n < pos && (peer = dot1x_peers_next(&intf->peers, peer)); n++)
    put32(peer->ssci.buf, ssci--);

  /* do local station */
  put32(intf->ssci.buf, ssci--);

  /* do the rest of the peers */
  while ((peer = dot1x_peers_next(&intf->peers, peer)))
    put32(peer->ssci.buf, ssci--);
}

static void
put32(unsigned char *buf, uint32_t n)
{
  buf[0] = (n >> 24) & 0xff;
  buf[1] = (n >> 16) & 0xff;
  buf[2] = (n >> 8) & 0xff;
  buf[3] = n & 0xff;
}

void
dot1x_set_suspended_while(Dot1xPort *port, unsigned value)
{
  if (port->suspended_while == value)
    return;

  port->suspended_while = value;

  dot1x_debug_mka_cp(
          port, "suspendedWhile = %u", port->suspended_while);
  port->intf->cp_changed = 1;
}

void
dot1x_set_suspend_on_request(Dot1xPort *port, unsigned value)
{
  if (port->suspend_on_request == value)
    return;

  port->suspend_on_request = value;
  dot1x_debug_mka_cp(
          port, "suspendOnRequest = %s", dot1x_bool(port->suspend_on_request));
  port->intf->cp_changed = 1;
}
