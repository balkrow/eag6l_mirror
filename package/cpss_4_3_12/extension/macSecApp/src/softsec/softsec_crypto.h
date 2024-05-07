/**

   Crypto functions used by the software MACsec protocol.

   File: softsec_crypto.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc., all rights reserved.

*/

#ifndef _SOFTSEC_CRYPTO_H
#define _SOFTSEC_CRYPTO_H

#include "softsec_platform.h"
#include "macsec_types.h"

/** Pointer and length for reading a byte string. */
typedef struct {
  /** Pointer to the first byte. */
  const unsigned char *buf;
  /** Number of bytes to read. */
  unsigned len;
} SoftsecCryptoReadBytes;

/** Pointer and length for writing a byte string. */
typedef struct {
  /** Pointer to the first byte. */
  unsigned char *buf;
  /** Number of bytes to write. */
  unsigned len;
} SoftsecCryptoWriteBytes;

/** A 96-bit AES-GCM initialization vector. */
typedef struct {
  /** Buffer containing the initialization vector. */
  unsigned char buf[12];
} SoftsecCryptoIv;

/** A 128-bit AES-GCM authentication tag. */
typedef struct {
  /** Buffer containing the authentication tag. */
  unsigned char buf[16];
} SoftsecCryptoTag;

#ifdef SOFTSEC_SOFTWARE_CRYPTO
/** Initialize an AES-GCM authenticated encryption/decryption context
    with a key.

    @param context
    The AES-GCM context to initialize.

    @param key
    Pointer to a 128-bit or 256-bit AES key, accessed during the call
    only.
*/
void
softsec_crypto_init(
  SoftsecCryptoContext *context,
  const MacsecKey *key);

/** Uninitialize and zeroize an AES-GCM authenticated
    encryption/decryption context.

    @param context
    The AES-GCM context to uninitialize.
*/
void
softsec_crypto_uninit(
  SoftsecCryptoContext *context);

/** Perform AES-GCM authenticated encryption.

    @param context
    Pointer to an initialized AES-GCM context.

    @param ciphertext
    Pointer and length of a location into which encrypted data is
    stored.

    @param tag
    Pointer to an object into which the resulting 128-bit AES-GCM
    authentication tag is stored.

    @param iv
    Pointer to a 96-bit AES-GCM initialization vector.

    @param plaintext
    Pointer and length of a location from which data to encrypt is
    read. Length must equal the length of the ciphertext.

    @param aad
    Pointer and length of a location from which additional
    authenticated data (AAD) is read.

    @description
    This function performs AES-GCM authenticated encryption using the
    key with which context was initialized using softsec_crypto_init()
    and the initialization vector iv.
*/
void
softsec_crypto_encrypt(
  SoftsecCryptoContext *context,
  SoftsecCryptoWriteBytes *ciphertext,
  SoftsecCryptoTag *tag,
  const SoftsecCryptoIv *iv,
  SoftsecCryptoReadBytes *plaintext,
  SoftsecCryptoReadBytes *aad);

/** Perform AES-GCM authenticated decryption.

    @param context
    Pointer to an initialized AES-GCM context.

    @param authentic
    Pointer to a variable into which the authentication tag
    verification result is stored, with nonzero value indicating
    successful verification.

    @param plaintext
    Pointer and length of a location into which decrypted data is
    stored.

    @param iv
    Pointer to a 96-bit AES-GCM initialization vector.

    @param ciphertext
    Pointer and length of a location from which data to decrypt is
    read. Length must equal the length of the plaintext.

    @param aad
    Pointer and length of a location from which additional
    authenticated data (AAD) is read.

    @param tag
    Pointer to an object from which a 128-bit AES-GCM authentication
    tag to verify is read.

    @description
    This function performs AES-GCM authenticated decryption using the
    key with which context was initialized using softsec_crypto_init()
    and the initialization vector iv.
*/
void
softsec_crypto_decrypt(
  SoftsecCryptoContext *context,
  unsigned *authentic,
  SoftsecCryptoWriteBytes *plaintext,
  const SoftsecCryptoIv *iv,
  SoftsecCryptoReadBytes *ciphertext,
  SoftsecCryptoReadBytes *aad,
  const SoftsecCryptoTag *tag);
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

#endif /* _SOFTSEC_CRYPTO_H */
