/**

   802.1X-2010 internal utility functions.

   File: dot1xpae_util.h

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_UTIL_H
#define _DOT1XPAE_UTIL_H

#include "dot1xpae_types.h"
#include "dot1xpae_crypto.h"
#include <stdarg.h>

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

/*
 * Get pointer to the most appropriate network to use for
 * authentication, i.e. check networks corresponding to selectedNID,
 * requestedNID and connectedNID in that order and take the first
 * non-NULL one. If all are null then take the default network.
 */
Dot1xNetwork *
dot1x_choose_network(Dot1xPort *port);

/*
 * Run a function on the real port of and all virtual ports of the
 * interface.
*/
void
dot1x_foreach_port(Dot1xIntf *intf, void (*func)(Dot1xPort *port));

/*
 * If virtual ports are disabled or peer_address is NULL, get a
 * reference to the real port of the interface. If virtual ports are
 * enabled and peer_address is not NULL, get a reference to the
 * virtual port of the peer object associated with the specified peer
 * address, increasing the reference count of the peer object. If no
 * such peer object exists then create one. If no more peer objects
 * can be created then return NULL.
 */
Dot1xPort *
dot1x_refer_port(Dot1xIntf *intf, const MacsecAddress *peer_address);

/*
 * Get an additional reference to a real or virtual port. In case of a
 * real port, has no effect. In case of a virtual port, increases the
 * reference count of the containing peer object.
 */
void
dot1x_reref_port(Dot1xPort *port);

/*
 * Release reference to a real or virtual port. In case of a real
 * port, has no effect. In case of a virtual port, decreases the
 * reference count of the containing peer object and destroys it after
 * the last reference.
 */
void
dot1x_unref_port(Dot1xPort *port);

/*
 * Get counted reference to a peer object associated with the given
 * MAC address. If no such peer object exists then create one and
 * depending on the requested type associate it with the real port or
 * with a new virtual port. If no more peer objects can be created
 * then return NULL.
 */
Dot1xPeer *
dot1x_refer_peer(
  Dot1xIntf *intf, Dot1xPeerType type, const MacsecAddress *peer_address);

Dot1xPeer *
dot1x_refer_peer_portid(
        Dot1xIntf *intf,
        Dot1xPeerType type,
        const MacsecAddress *peer_address,
        int portid);

/*
 * Release a counted reference to a peer object. Destroy the object
 * when releasing the last reference.
 */
void
dot1x_unref_peer(Dot1xIntf *intf, Dot1xPeer *peer);

/*
 * Increase the reference count of a peer object by one.
 */
void
dot1x_reref_peer(Dot1xIntf *intf, Dot1xPeer *peer);

/*
 * Create a `len'-byte derived key into `buf' using the 802.1X-2010
 * key derivation function (KDF). The only allowed values of `len' are
 * 16 and 32. The syntax in the specification is the following:
 *
 * KDF(Key, Label, Context, Length)
 *
 * The `key' parameter of the function below corresponds to the key
 * kerivation key (Key) of the KDF, the `label' parameter corresponds
 * to Label and the function `context' is used to add the key context
 * information (Context) to the AES-CMAC object `cmac'. The Length
 * parameter of the KDF is given by `len'. The `arg' parameter is
 * passed unchanged to the context adding function.
 */
void
dot1x_derive_key(
  Dot1xIntf *intf, unsigned char *buf, unsigned len,
  const MacsecKey *key, const char *label,
  void (*context)(void *arg, Dot1xCryptoCmac *cmac), void *arg);

/*
 * Get an EAPOL multicast address from an multicast group identifier.
*/
const MacsecAddress *
dot1x_eapol_address(Dot1xEapolGroup group);

/*
 * Macros for printing diagnostic messages of various categories.
 */

#define dot1x_error(intf, ...) \
dot1x_print((intf), DOT1X_MESSAGE_ERROR, __VA_ARGS__)
#define dot1x_error_port(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_ERROR, __VA_ARGS__)
#define dot1x_error_peer(peer, ...) \
dot1x_print_peer((peer), DOT1X_MESSAGE_ERROR, __VA_ARGS__)
#define dot1x_error_mka(mka, ...) \
dot1x_print_mka((mka), DOT1X_MESSAGE_ERROR, __VA_ARGS__)
#define dot1x_error_mka_peer(mkap, ...) \
dot1x_print_mka_peer((mkap), DOT1X_MESSAGE_ERROR, __VA_ARGS__)
#define dot1x_error_pacp(pacp, ...) \
dot1x_print_pacp((pacp), DOT1X_MESSAGE_ERROR, __VA_ARGS__)

#define dot1x_debug_discard(intf, ...) \
dot1x_print((intf), DOT1X_MESSAGE_DISCARD, __VA_ARGS__)
#define dot1x_debug_discard_mka(mka, ...) \
dot1x_print_mka((mka), DOT1X_MESSAGE_DISCARD, __VA_ARGS__)

#define dot1x_debug_port(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_PORT, __VA_ARGS__)

#define dot1x_debug_network(network, ...) \
dot1x_print_network((network), DOT1X_MESSAGE_NETWORK, __VA_ARGS__)
#define dot1x_debug_network_intf(intf, ...) \
dot1x_print((intf), DOT1X_MESSAGE_NETWORK, __VA_ARGS__)
#define dot1x_debug_network_port(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_NETWORK, __VA_ARGS__)

#define dot1x_debug_peer(peer, ...) \
dot1x_print_peer((peer), DOT1X_MESSAGE_PEER, __VA_ARGS__)

#define dot1x_debug_cak_port(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_CAK, __VA_ARGS__)
#define dot1x_debug_cak(cak, ...) \
dot1x_print_cak((cak), DOT1X_MESSAGE_CAK, __VA_ARGS__)

#define dot1x_debug_sak(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_SAK, __VA_ARGS__)

#define dot1x_debug_key_server(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_KEY_SERVER, __VA_ARGS__)
#define dot1x_debug_key_server_mka(mka, ...) \
dot1x_print_mka((mka), DOT1X_MESSAGE_KEY_SERVER, __VA_ARGS__)
#define dot1x_debug_key_server_mka_peer(mkap, ...) \
dot1x_print_mka_peer((mkap), DOT1X_MESSAGE_KEY_SERVER, __VA_ARGS__)

#define dot1x_debug_eapol(intf, ...) \
dot1x_print((intf), DOT1X_MESSAGE_EAPOL, __VA_ARGS__)

#define dot1x_debug_eap(intf, ...) \
dot1x_print((intf), DOT1X_MESSAGE_EAP, __VA_ARGS__)

#define dot1x_debug_mka(mka, ...) \
dot1x_print_mka((mka), DOT1X_MESSAGE_MKA, __VA_ARGS__)

#define dot1x_debug_announcement(intf, ...) \
dot1x_print((intf), DOT1X_MESSAGE_ANNOUNCEMENT, __VA_ARGS__)

#define dot1x_debug_pacp(pacp, ...) \
dot1x_print_pacp((pacp), DOT1X_MESSAGE_PACP, __VA_ARGS__)

#define dot1x_debug_mka_actor(mka, ...) \
dot1x_print_mka((mka), DOT1X_MESSAGE_MKA_ACTOR, __VA_ARGS__)

#define dot1x_debug_mka_peer(mkap, ...) \
dot1x_print_mka_peer((mkap), DOT1X_MESSAGE_MKA_PEER, __VA_ARGS__)

#define dot1x_debug_cp(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_CP, __VA_ARGS__)

#define dot1x_debug_control(intf, ...) \
dot1x_print((intf), DOT1X_MESSAGE_CONTROL, __VA_ARGS__)
#define dot1x_debug_control_network(network, ...) \
dot1x_print_network((network), DOT1X_MESSAGE_CONTROL, __VA_ARGS__)

#define dot1x_debug_logon_pacp(intf, ...) \
dot1x_print((intf), DOT1X_MESSAGE_LOGON_PACP, __VA_ARGS__)

#define dot1x_debug_logon_pacp_port(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_LOGON_PACP, __VA_ARGS__)

#define dot1x_debug_logon_mka(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_LOGON_MKA, __VA_ARGS__)

#define dot1x_debug_logon_cp(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_LOGON_CP, __VA_ARGS__)

#define dot1x_debug_pacp_eapol(pacp, ...) \
dot1x_print_pacp((pacp), DOT1X_MESSAGE_PACP_EAPOL, __VA_ARGS__)

#define dot1x_debug_pacp_eap(pacp, ...) \
dot1x_print_pacp((pacp), DOT1X_MESSAGE_PACP_EAP, __VA_ARGS__)

#define dot1x_debug_mka_cp(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_MKA_CP, __VA_ARGS__)

#define dot1x_debug_mka_secy(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_MKA_SECY, __VA_ARGS__)
#define dot1x_debug_mka_secy_peer(peer, ...) \
dot1x_print_peer((peer), DOT1X_MESSAGE_MKA_SECY, __VA_ARGS__)

#define dot1x_debug_cp_secy(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_CP_SECY, __VA_ARGS__)

#define dot1x_debug_cp_client(port, ...) \
dot1x_print_port((port), DOT1X_MESSAGE_CP_CLIENT, __VA_ARGS__)

/*
 * If messages of the specified category are enabled, append newline
 * to the message and print it using dot1x_vprintf().
 */
void
dot1x_print(Dot1xIntf *intf, int category, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4)));

/*
 * Like dot1x_print() but prefix the message with default or announced
 * network identification.
 */
void
dot1x_print_network(Dot1xNetwork *network, int category, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4)));

/*
 * Like dot1x_print() but prefix the message with real or virtual
 * port identification.
 */
void
dot1x_print_port(Dot1xPort *port, int category, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4)));

/*
 * Like dot1x_print() but prefix the message with real or virtual
 * peer identification.
 */
void
dot1x_print_peer(Dot1xPeer *peer, int category, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4)));

/*
 * Like dot1x_print() but prefix the message with MKA instance
 * identification.
 */
void
dot1x_print_mka(Dot1xMka *mka, int category, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4)));

/*
 * Like dot1x_print() but prefix the message with MKA peer
 * identification.
 */
void
dot1x_print_mka_peer(Dot1xMkaPeer *mkap, int category, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4)));

/*
 * Like dot1x_print() but prefix the message with PACP instance
 * identification.
 */
void
dot1x_print_pacp(Dot1xPacp *pacp, int category, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4)));

/*
 * Like dot1x_print() but prefix the message with CKN.
 */
void
dot1x_print_cak(Dot1xCak *cak, int category, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4)));

/*
 * Return 1 if the specified message category is enabled, 0 otherwise.
 */
static inline int
dot1x_debug_enabled(const Dot1xIntf *intf, int category)
{
  return !!(intf->messages & 1 << category);
}

/*
 * Print buffer contents in hex, 16 bytes per line using
 * dot1x_vprintf().
 */
void
dot1x_print_hex(Dot1xIntf *intf, const unsigned char *buf, unsigned len);

/*
 * Like dot1x_vprintf() but with a variable argument list.
 */
void
dot1x_printf(Dot1xIntf *intf, const char *fmt, ...)
     __attribute__ ((format (printf, 2, 3)));

/*
 * Format a diagnostic message using the vprintf-style format string
 * `fmt' and arguments `ap' and append it to the message buffer of the
 * interface. Output each newline-terminated part of the message
 * buffer using a call to dot1x_platform_log().
 */
void
dot1x_vprintf(Dot1xIntf *intf, const char *fmt, va_list ap);

/*
 * Generic string formatting buffer for diagnostics.
 */
typedef char Dot1xFormatBuffer[97];

/*
 * Functions to write human-readable representations of various
 * objects into a formatting buffer. Output is truncated if the
 * representation would exceed buffer size but always null-terminated.
 */

const char *
dot1x_format_network(Dot1xFormatBuffer *fbuf, const Dot1xNetwork *netw);

const char *
dot1x_format_pacp(Dot1xFormatBuffer *fbuf, const Dot1xPacp *pacp);

const char *
dot1x_format_cak(Dot1xFormatBuffer *fbuf, const Dot1xCak *cak);

const char *
dot1x_format_sci(Dot1xFormatBuffer *fb, const MacsecSci *sci);

const char *
dot1x_format_address(Dot1xFormatBuffer *fb, const MacsecAddress *address);

const char *
dot1x_format_ckn(Dot1xFormatBuffer *fb, const Dot1xCkn *ckn);

const char *
dot1x_format_key(Dot1xFormatBuffer *fb, const MacsecKey *key);

const char *
dot1x_format_wrap(Dot1xFormatBuffer *fb, const Dot1xCryptoWrap *wrap);

const char *
dot1x_format_icv(Dot1xFormatBuffer *fb, const Dot1xIcv *icv);

const char *
dot1x_format_key_identifier(
  Dot1xFormatBuffer *fb, const Dot1xKeyIdentifier *ki);

const char *
dot1x_format_member_identifier(
  Dot1xFormatBuffer *fb, const Dot1xMemberIdentifier *mi);

const char *
dot1x_format_nid(Dot1xFormatBuffer *fb, const Dot1xNid *nid);

const char *
dot1x_format_kmd(Dot1xFormatBuffer *fb, const Dot1xKmd *kmd);

const char *
dot1x_format_utf8(
  Dot1xFormatBuffer *fb, const unsigned char *buf, unsigned len);

const char *
dot1x_format_vprintf(Dot1xFormatBuffer *fb, const char *fmt, va_list ap);

/*
 * If `len' is zero, write the word '<empty>' into the formatting
 * buffer. If `len' is nonzero, behave like
 * dot1x_format_hex(). Return pointer to the null-terminated
 * string in the buffer.
 */
const char *
dot1x_format_buf(
  Dot1xFormatBuffer *fb, const unsigned char *buf, unsigned len);

/*
 * Write `len' bytes starting at `buf' in upper-case hexadecimal
 * notation into the formatting buffer and return pointer to the
 * string in the buffer. The two-digit hex numbers are separated by
 * colons and the string is null-terminated. If the output would
 * exceed the size of the formatting buffer then truncate the
 * hexadecimal representation but always null-terminate.
 */
const char *
dot1x_format_hex(
  Dot1xFormatBuffer *fb, const unsigned char *buf, unsigned len);

/*
 * Functions to convert (mostly) integer values to constant strings.
 */

const char *
dot1x_pacp_side(Dot1xPacpSide *side);

const char *
dot1x_eapol_group(Dot1xEapolGroup group);

const char *
dot1x_pacp_state(int state);

const char *
dot1x_cp_state(int state);

const char *
dot1x_connect(int connect);

const char *
dot1x_use_eap(Dot1xUseEap n);

const char *
dot1x_unauth_allowed(Dot1xUnauthAllowed n);

const char *
dot1x_unsecured_allowed(Dot1xUnsecuredAllowed n);

const char *
dot1x_access_status(Dot1xAccessStatus n);

const char *
dot1x_unauthenticated_access(Dot1xUnauthenticatedAccess n);

const char *
dot1x_validate_frames(MacsecValidateFrames n);

const char *
dot1x_cipher_suite(MacsecCipherSuite n);

const char *
dot1x_macsec_capability(MacsecCapability n);

const char *
dot1x_mka_algorithm(int n);

const char *
dot1x_confidentiality_offset(MacsecConfidentialityOffset n);

const char *
dot1x_bool(int value);

const char *
dot1x_cak_type(Dot1xCakType type);

const char *
dot1x_eapol_type(unsigned type);

/*
 * Convert cipher suite identifier to pointer to a 8-byte reference
 * number. Return 1 if successful.
 */
int
dot1x_get_cipher_suite_refnr(
  const unsigned char (**refnr)[8], MacsecCipherSuite cs);

/*
 * Convert 8-byte cipher suite reference number to identifier. Return
 * 1 if successful.
 */
int
dot1x_get_cipher_suite_ident(
  MacsecCipherSuite *cs, unsigned char (*refnr)[8]);

/*
 * Compare two buffers using memcmp(), using min(len1, len2) bytes. If
 * memcmp() returns nonzero, return that result. Otherwise, return
 * len1 - len2.
 */
static inline int
dot1x_bufcmp(
  const unsigned char *buf1, unsigned len1,
  const unsigned char *buf2, unsigned len2)
{
  int lendiff, minlen, n;

  if ((lendiff = len1 - len2) < 0)
    minlen = len1;
  else
    minlen = len2;

  if ((n = memcmp(buf1, buf2, minlen)))
    return n;
  else
    return lendiff;
}


/*
 * Copy a buffer to another one and set the length of the latter
 * accordingly.
 */
static inline void
dot1x_bufcpy(
  unsigned char *dstbuf, unsigned *dstlen,
  const unsigned char *srcbuf, unsigned srclen)
{
  memcpy(dstbuf, srcbuf, srclen);
  *dstlen = srclen;
}

/*
 * Comparison and copy functions for types with a buffer pointer and
 * length.
 */

static inline int
dot1x_ckncmp(const Dot1xCkn *obj1, const Dot1xCkn *obj2)
{
  return dot1x_bufcmp(obj1->buf, obj1->len, obj2->buf, obj2->len);
}

static inline void
dot1x_ckncpy(Dot1xCkn *dst, const Dot1xCkn *src)
{
  return dot1x_bufcpy(dst->buf, &dst->len, src->buf, src->len);
}

static inline int
dot1x_nidcmp(const Dot1xNid *obj1, const Dot1xNid *obj2)
{
  return dot1x_bufcmp(obj1->buf, obj1->len, obj2->buf, obj2->len);
}

static inline void
dot1x_nidcpy(Dot1xNid *dst, const Dot1xNid *src)
{
  return dot1x_bufcpy(dst->buf, &dst->len, src->buf, src->len);
}

static inline int
dot1x_kmdcmp(const Dot1xKmd *obj1, const Dot1xKmd *obj2)
{
  return dot1x_bufcmp(obj1->buf, obj1->len, obj2->buf, obj2->len);
}

static inline void
dot1x_kmdcpy(Dot1xKmd *dst, const Dot1xKmd *src)
{
  return dot1x_bufcpy(dst->buf, &dst->len, src->buf, src->len);
}

static inline void
dot1x_keycpy(MacsecKey *dst, const MacsecKey *src)
{
  return dot1x_bufcpy(dst->buf, &dst->len, src->buf, src->len);
}

static inline void
dot1x_adcpy(MacsecAuthData *dst, const MacsecAuthData *src)
{
  return dot1x_bufcpy(dst->buf, &dst->len, src->buf, src->len);
}

static inline unsigned
dot1x_to_secs(unsigned ticks)
{
    return (ticks + DOT1X_TICKS_PER_SECOND - 1) / DOT1X_TICKS_PER_SECOND;
}

static inline unsigned
dot1x_to_ticks(unsigned secs)
{
    return secs * DOT1X_TICKS_PER_SECOND;
}
#include <stdio.h>
/*
 * Set suspendedWhile variable of port.
 */
void
dot1x_set_suspended_while(Dot1xPort *port, unsigned value);

void
dot1x_suspension_start(
  Dot1xPort *port,
  Dot1xMka *mka,
  unsigned suspended_while,
  const char *reason);

void
dot1x_suspension_stop(
  Dot1xPort *port,
  const char *reason);

#endif /* _DOT1XPAE_UTIL_H */
