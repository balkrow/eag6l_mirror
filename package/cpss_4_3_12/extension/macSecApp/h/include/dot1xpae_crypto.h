/**

   Crypto functions used by the 802.1X protocol.

   File: dot1xpae_crypto.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_CRYPTO_H
#define _DOT1XPAE_CRYPTO_H

/*
 * Get implentation-dependent definitions of Dot1xCryptoRandom and
 * Dot1xCryptoCmac.
 */
#include "dot1xpae_crypto_types.h"

#include "macsec_types.h"
#include <stddef.h>

/** 16-byte message authentication code (MAC) from AES-CMAC. */
typedef struct {
  unsigned char buf[16];
} Dot1xCryptoCmacValue;

/** AES key wrap of a 128-bit or 256-bit key. */
typedef struct {
  /** Buffer containing the data. */
  unsigned char buf[40];
  /** Length of data in bytes (24 or 40, equals plaintext key length
      plus 8 octets). */
  unsigned int len;
} Dot1xCryptoWrap;

/** Initialize a random number generator (RNG) context.

    @param random
    Pointer to the RNG context to initialize.

    @description
    This function sets up a RNG for subsequent random number retrieval
    using dot1x_crypto_random_generate().
*/
void
dot1x_crypto_random_init(
  Dot1xCryptoRandom *random);

/** Uninitialize a random number generator context.

    @param random
    Pointer to the RNG context to uninitialize.
 */
void
dot1x_crypto_random_uninit(
  Dot1xCryptoRandom *random);

/** Get random bytes from a random number generator.

    @param random
    Pointer to a RNG context.

    @param buf
    Pointer to a buffer into which random bytes should be stored.

    @param len
    Number of random bytes required.

    @return
    1 if successful, 0 if there was an RNG failure.
 */
int
dot1x_crypto_random_generate(
  Dot1xCryptoRandom *random,
  unsigned char *buf,
  unsigned len);

/** Initialize an AES-CMAC context with a key.

    @param cmac
    Pointer to the AES-CMAC context to initialize.

    @param key
    Pointer to a 128-bit or 256-bit AES key, accessed during the call
    only.

    @description
    This function prepares the AES-CMAC context for submitting input
    data using zero or more calls to dot1x_crypto_cmac_update().
 */
void
dot1x_crypto_cmac_start(
  Dot1xCryptoCmac *cmac,
  const MacsecKey *key);

/** Update an AES-CMAC context with additional input data.

    @param cmac
    Pointer to an AES-CMAC context.

    @param buf
    Pointer to a buffer containing input data.

    @param len
    Length of input data in bytes.

    @description
    This function processes the given input data using the AES-CMAC
    context and updates the state of AES-CMAC calculation in the
    context. This function can be called zero or more times between a
    call to dot1x_crypto_cmac_start() and a call to
    dot1x_crypto_cmac_finish().
 */
void
dot1x_crypto_cmac_update(
  Dot1xCryptoCmac *cmac,
  const unsigned char *buf,
  unsigned len);

/** Finalize an AES-CMAC context and get a message authentication code
    (MAC).

    @param cmac
    Pointer to an AES-CMAC context.

    @param value
    Pointer to a MAC object into which the calculated MAC is stored.

    @description
    This function finishes the AES-CMAC calculation, stores the MAC
    value in the object provided and clears the context object,
    including material derived from the key previously passed to a
    dot1x_crypto_cmac_start(). A new call to dot1x_crypto_cmac_start()
    is required before using the AES-CMAC context again.
 */
void
dot1x_crypto_cmac_finish(
  Dot1xCryptoCmac *cmac,
  Dot1xCryptoCmacValue *value);

/** Make an AES key wrap of a plaintext key.

    @param wrap
    Pointer to an AES key wrap object into which the wrapped key is
    stored.

    @param kek
    Pointer to a 128-bit or 256-bit AES key used as the key-encryption
    key (KEK).

    @param plain
    Pointer to a 128-bit or 256-bit AES key to wrap.
 */
void
dot1x_crypto_wrap(
  Dot1xCryptoWrap *wrap,
  const MacsecKey *kek,
  const MacsecKey *plain);

/** Extract a plaintext key from an AES key wrap object.

    @param wrap
    Pointer to an AES key wrap object fromw which the wrapped key is
    to be extracted.

    @param kek
    Pointer to a 128-bit or 256-bit AES key used as the key-encryption
    key (KEK).

    @param plain
    Pointer to a 128-bit or 256-bit AES key object into which the
    unwrapped plaintext key is stored.
 */
void
dot1x_crypto_unwrap(
  const Dot1xCryptoWrap *wrap,
  const MacsecKey *kek,
  MacsecKey *plain);

#endif /* _DOT1XPAE_CRYPTO_H */
